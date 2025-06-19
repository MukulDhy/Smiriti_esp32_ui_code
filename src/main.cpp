#include <WiFi.h>
#include "ui/ui.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <Wire.h>
#include "tasks/wifi.h"
#include "tasks/time.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// Touchscreen pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Screen dimensions
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Memory thresholds
#define MIN_FREE_HEAP 8192
#define CRITICAL_HEAP 4096

// Task stack sizes
#define UI_TASK_STACK_SIZE 12288
#define NETWORK_TASK_STACK_SIZE 8192
#define MQTT_TASK_STACK_SIZE 8192

// LVGL configuration
#define DRAW_BUF_MULTIPLIER 10
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / DRAW_BUF_MULTIPLIER * (LV_COLOR_DEPTH / 8))

// Configuration
const char *WIFI_SSID = "Mukuldhy";
const char *WIFI_PASS = "12345678";

struct Config
{
  const char *MQTT_USERNAME = "mukulmqtt";
  const char *MQTT_PASSWORD = "Mukul@jaat123";
} config;

// MQTT Configuration
const char *mqtt_server = "02ed6b84181647639b35d467c00afbd9.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char *device_id = "2113";
char status_topic[50];
char command_topic[50];
char record_topic[50];

// Connection timeouts
#define WIFI_TIMEOUT_MS 15000
#define MQTT_TIMEOUT_MS 10000
#define MQTT_RETRY_INTERVAL 10000
#define STATUS_UPDATE_INTERVAL 30000
#define DNS_TIMEOUT_MS 5000

// Hardware objects
WiFiClientSecure espClientSecure;
PubSubClient client(espClientSecure);
SPIClass touchscreenSPI(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Memory management
uint8_t *draw_buf = nullptr;

// Task handles
TaskHandle_t uiTaskHandle = NULL;
TaskHandle_t networkTaskHandle = NULL;
TaskHandle_t mqttTaskHandle = NULL;

// Synchronization
SemaphoreHandle_t lvglMutex;
QueueHandle_t uiUpdateQueue;

// Performance monitoring
struct SystemStats
{
  unsigned long lastHeapCheck = 0;
  unsigned long lastStatusUpdate = 0;
  unsigned long lastMqttReconnect = 0;
  uint32_t minFreeHeap = UINT32_MAX;
  bool lowMemoryWarning = false;
  bool mqttConnected = false;
  unsigned long lastMqttMessage = 0; // Track last received message
};
SystemStats systemStats;

// UI Update structure
struct UIUpdate
{
  char title[32];
  char description[64];
  bool isAlert;
};

// Calibration values
#define TOUCH_MIN_X 493
#define TOUCH_MAX_X 3545
#define TOUCH_MIN_Y 418
#define TOUCH_MAX_Y 3549

// Forward declarations
void uiTask(void *parameter);
void networkTask(void *parameter);
void mqttTask(void *parameter);
void checkMemoryUsage();
bool validateBufferSize();
void handleLowMemory();
void optimizedReconnect();

// Logging
void log_print(lv_log_level_t level, const char *buf)
{
  if (ESP.getFreeHeap() > CRITICAL_HEAP)
  {
    Serial.println(buf);
  }
}

// FIXED: Enhanced UI update function with better error handling
void updateUIElements(const char *title, const char *description)
{
  if (!title || !description || !lvglMutex)
  {
    Serial.println("ERROR: Invalid parameters for UI update");
    return;
  }

  // Additional validation
  if (strlen(title) > 100 || strlen(description) > 200)
  {
    Serial.println("ERROR: UI update strings too long");
    return;
  }

  Serial.printf("DEBUG: Attempting UI update - Title: %s, Desc: %s\n", title, description);

  // Try to take mutex with timeout
  if (xSemaphoreTake(lvglMutex, pdMS_TO_TICKS(100)) == pdTRUE) // Increased timeout
  {
    // Check display validity
    lv_display_t *disp = lv_display_get_default();
    if (!disp)
    {
      Serial.println("ERROR: No default display available");
      xSemaphoreGive(lvglMutex);
      return;
    }

    // Wait for rendering to complete
    int renderWaitCount = 0;
    while (disp->rendering_in_progress && renderWaitCount < 20) // Increased wait
    {
      xSemaphoreGive(lvglMutex);
      vTaskDelay(pdMS_TO_TICKS(10));
      renderWaitCount++;

      if (xSemaphoreTake(lvglMutex, pdMS_TO_TICKS(100)) != pdTRUE)
      {
        Serial.println("ERROR: Failed to retake mutex after render wait");
        return;
      }
      disp = lv_display_get_default();
    }

    if (disp->rendering_in_progress)
    {
      Serial.println("WARNING: Still rendering - skipping UI update");
      xSemaphoreGive(lvglMutex);
      return;
    }

    // Check memory before proceeding
    if (ESP.getFreeHeap() > MIN_FREE_HEAP)
    {
      lv_obj_t *current_screen = lv_scr_act();

      // Switch to reminder alert screen if not already there
      if (current_screen != ui_screen_reminderalert)
      {
        Serial.println("DEBUG: Switching to reminder alert screen");
        _ui_screen_change(&ui_screen_reminderalert, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, &ui_screen_reminderalert_screen_init);

        // Give time for screen change
        xSemaphoreGive(lvglMutex);
        vTaskDelay(pdMS_TO_TICKS(300));

        // Retake mutex
        if (xSemaphoreTake(lvglMutex, pdMS_TO_TICKS(100)) != pdTRUE)
        {
          Serial.println("ERROR: Failed to retake mutex after screen change");
          return;
        }
      }

      // Update labels with validation
      if (ui_reminderalert_label_label33 && lv_obj_is_valid(ui_reminderalert_label_label33))
      {
        lv_obj_t *parent = lv_obj_get_screen(ui_reminderalert_label_label33);
        if (parent == lv_scr_act())
        {
          lv_label_set_text(ui_reminderalert_label_label33, description);
          Serial.println("DEBUG: Updated description label");
        }
        else
        {
          Serial.println("WARNING: Description label not on current screen");
        }
      }
      else
      {
        Serial.println("ERROR: Description label is invalid or null");
      }

      if (ui_reminderalert_label_label29 && lv_obj_is_valid(ui_reminderalert_label_label29))
      {
        lv_obj_t *parent = lv_obj_get_screen(ui_reminderalert_label_label29);
        if (parent == lv_scr_act())
        {
          lv_label_set_text(ui_reminderalert_label_label29, title);
          Serial.println("DEBUG: Updated title label");
        }
        else
        {
          Serial.println("WARNING: Title label not on current screen");
        }
      }
      else
      {
        Serial.println("ERROR: Title label is invalid or null");
      }

      Serial.println("SUCCESS: UI update completed");
    }
    else
    {
      Serial.printf("ERROR: Low memory - skipping UI update. Free: %u\n", ESP.getFreeHeap());
    }

    xSemaphoreGive(lvglMutex);
  }
  else
  {
    Serial.println("ERROR: Failed to take mutex for UI update");
  }
}

// Touchscreen handling
void touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  static TS_Point lastValidPoint = {0, 0, 0};
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastReadTime < 20)
  {
    data->point.x = lastValidPoint.x;
    data->point.y = lastValidPoint.y;
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  if (touchscreen.tirqTouched() && touchscreen.touched())
  {
    TS_Point p = touchscreen.getPoint();

    if (p.x >= TOUCH_MIN_X && p.x <= TOUCH_MAX_X &&
        p.y >= TOUCH_MIN_Y && p.y <= TOUCH_MAX_Y)
    {
      lastValidPoint = p;
      data->point.x = map(p.x, TOUCH_MIN_X, TOUCH_MAX_X, 0, SCREEN_WIDTH);
      data->point.y = map(p.y, TOUCH_MIN_Y, TOUCH_MAX_Y, 0, SCREEN_HEIGHT);
      data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
      data->state = LV_INDEV_STATE_RELEASED;
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }

  lastReadTime = currentTime;
}

// Check LVGL state
bool isLVGLSafeToUpdate()
{
  lv_display_t *disp = lv_display_get_default();
  if (!disp)
  {
    return false;
  }
  return !disp->rendering_in_progress;
}

// FIXED: Enhanced MQTT callback with better JSON parsing and debugging
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.printf("DEBUG: MQTT message received - Topic: %s, Length: %u\n", topic, length);

  // Basic validation
  if (!topic || !payload || length == 0 || length > 500)
  {
    Serial.println("ERROR: Invalid MQTT message parameters");
    return;
  }

  // Only process our command topic
  if (strcmp(topic, command_topic) != 0)
  {
    Serial.printf("DEBUG: Ignoring message from topic: %s\n", topic);
    return;
  }

  // Update last message time
  systemStats.lastMqttMessage = millis();

  // Create null-terminated string from payload
  char payloadStr[501];
  size_t copyLen = min(length, (unsigned int)500);
  memcpy(payloadStr, payload, copyLen);
  payloadStr[copyLen] = '\0';

  Serial.printf("DEBUG: Payload content: %s\n", payloadStr);

  // Check if LVGL is safe to update
  if (!isLVGLSafeToUpdate())
  {
    Serial.println("WARNING: LVGL not safe - deferring callback");
    vTaskDelay(pdMS_TO_TICKS(50));
    if (!isLVGLSafeToUpdate())
    {
      Serial.println("ERROR: LVGL still not safe - skipping callback");
      return;
    }
  }

  // IMPROVED JSON parsing with ArduinoJson library
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, payloadStr);

  if (error)
  {
    Serial.printf("ERROR: JSON parsing failed: %s\n", error.c_str());

    // Fallback to manual parsing
    char *titleStart = strstr(payloadStr, "\"title\":\"");
    char *descStart = strstr(payloadStr, "\"description\":\"");

    UIUpdate uiUpdate = {"Reminder", "Check your schedule", false};

    if (titleStart)
    {
      titleStart += 9;
      char *titleEnd = strchr(titleStart, '"');
      if (titleEnd && (titleEnd - titleStart) < 31)
      {
        strncpy(uiUpdate.title, titleStart, titleEnd - titleStart);
        uiUpdate.title[titleEnd - titleStart] = '\0';
      }
    }

    if (descStart)
    {
      descStart += 14;
      char *descEnd = strchr(descStart, '"');
      if (descEnd && (descEnd - descStart) < 63)
      {
        strncpy(uiUpdate.description, descStart, descEnd - descStart);
        uiUpdate.description[descEnd - descStart] = '\0';
      }
    }

    Serial.printf("DEBUG: Manual parsing - Title: %s, Desc: %s\n", uiUpdate.title, uiUpdate.description);

    // Send to UI queue
    if (xQueueSend(uiUpdateQueue, &uiUpdate, pdMS_TO_TICKS(100)) != pdTRUE)
    {
      Serial.println("ERROR: UI queue full - dropping update");
    }
    else
    {
      Serial.println("SUCCESS: UI update queued (manual parsing)");
    }
  }
  else
  {
    // Successful JSON parsing
    UIUpdate uiUpdate = {"Reminder", "Check your schedule", false};

    if (doc.containsKey("title"))
    {
      const char *title = doc["title"];
      if (title && strlen(title) < 31)
      {
        strcpy(uiUpdate.title, title);
      }
    }

    if (doc.containsKey("description"))
    {
      const char *description = doc["description"];
      if (description && strlen(description) < 63)
      {
        strcpy(uiUpdate.description, description);
      }
    }

    Serial.printf("DEBUG: JSON parsing success - Title: %s, Desc: %s\n", uiUpdate.title, uiUpdate.description);

    // Send to UI queue
    if (xQueueSend(uiUpdateQueue, &uiUpdate, pdMS_TO_TICKS(100)) != pdTRUE)
    {
      Serial.println("ERROR: UI queue full - dropping update");
    }
    else
    {
      Serial.println("SUCCESS: UI update queued (JSON parsing)");
    }
  }
}

// FIXED: Event handlers with better error handling and debugging
void callAlertToBackend(lv_event_t *e)
{
  Serial.println("DEBUG: Alert button pressed");

  if (!systemStats.mqttConnected)
  {
    Serial.println("ERROR: MQTT not connected - cannot send alert");
    return;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("ERROR: WiFi not connected - cannot send alert");
    return;
  }

  // Create JSON document for alert
  StaticJsonDocument<200> alertDoc;
  alertDoc["device_id"] = device_id;
  alertDoc["status"] = "alert";
  alertDoc["type"] = "emergency";
  alertDoc["timestamp"] = millis();

  char alertBuffer[200];
  size_t alertSize = serializeJson(alertDoc, alertBuffer);

  Serial.printf("DEBUG: Sending alert: %s\n", alertBuffer);

  bool published = client.publish(status_topic, alertBuffer, alertSize);
  if (published)
  {
    Serial.println("SUCCESS: Alert sent to backend");
  }
  else
  {
    Serial.println("ERROR: Failed to publish alert");
  }
}

void sendRecordingRequestToBackend(lv_event_t *e)
{
  Serial.println("DEBUG: Recording button pressed");

  if (!systemStats.mqttConnected)
  {
    Serial.println("ERROR: MQTT not connected - cannot send recording request");
    return;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("ERROR: WiFi not connected - cannot send recording request");
    return;
  }

  // Create JSON document for recording request
  StaticJsonDocument<200> recordDoc;
  recordDoc["device_id"] = device_id;
  recordDoc["status"] = "record_request";
  recordDoc["timestamp"] = millis();

  char recordBuffer[200];
  size_t recordSize = serializeJson(recordDoc, recordBuffer);

  Serial.printf("DEBUG: Sending recording request: %s\n", recordBuffer);

  bool published = client.publish(record_topic, recordBuffer, recordSize);
  if (published)
  {
    Serial.println("SUCCESS: Recording request sent to backend");
  }
  else
  {
    Serial.println("ERROR: Failed to publish recording request");
  }
}

// Memory monitoring
void checkMemoryUsage()
{
  uint32_t freeHeap = ESP.getFreeHeap();

  if (freeHeap < systemStats.minFreeHeap)
  {
    systemStats.minFreeHeap = freeHeap;
  }

  if (freeHeap < CRITICAL_HEAP)
  {
    Serial.println("CRITICAL: Memory exhausted - restarting");
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP.restart();
  }
}

bool validateBufferSize()
{
  size_t requiredSize = DRAW_BUF_SIZE;
  size_t availableHeap = ESP.getMaxAllocHeap();

  if (requiredSize > availableHeap * 0.2)
  {
    Serial.printf("ERROR: Buffer too large: %u > %u\n", requiredSize, (uint32_t)(availableHeap * 0.2));
    return false;
  }
  return true;
}

// UI Task
void uiTask(void *parameter)
{
  uint32_t lastTick = millis();
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(25);

  UIUpdate uiUpdate;
  int consecutiveFailures = 0;
  bool renderingWarningShown = false;

  Serial.println("DEBUG: UI Task started");

  for (;;)
  {
    // Check memory before processing
    if (ESP.getFreeHeap() < CRITICAL_HEAP)
    {
      Serial.println("CRITICAL: Low memory in UI task");
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    // Handle UI updates from queue
    int updateCount = 0;
    while (xQueueReceive(uiUpdateQueue, &uiUpdate, 0) == pdTRUE && updateCount < 3)
    {
      Serial.printf("DEBUG: Processing UI update from queue - Title: %s\n", uiUpdate.title);
      updateUIElements(uiUpdate.title, uiUpdate.description);
      vTaskDelay(pdMS_TO_TICKS(100));
      updateCount++;
    }

    // Update LVGL
    if (xSemaphoreTake(lvglMutex, pdMS_TO_TICKS(30)) == pdTRUE)
    {
      lv_display_t *disp = lv_display_get_default();

      if (!disp)
      {
        Serial.println("ERROR: Display not available in UI task");
        xSemaphoreGive(lvglMutex);
        consecutiveFailures++;
      }
      else if (disp->rendering_in_progress)
      {
        if (!renderingWarningShown)
        {
          Serial.println("DEBUG: Skipping LVGL update - rendering in progress");
          renderingWarningShown = true;
        }
        xSemaphoreGive(lvglMutex);
      }
      else
      {
        renderingWarningShown = false;

        uint32_t currentTick = millis();
        uint32_t elapsed = currentTick - lastTick;

        if (elapsed > 0 && elapsed < 1000)
        {
          lv_tick_inc(elapsed);
        }
        lastTick = currentTick;

        lv_timer_handler();
        consecutiveFailures = 0;

        xSemaphoreGive(lvglMutex);
      }
    }
    else
    {
      consecutiveFailures++;
      if (consecutiveFailures % 20 == 0)
      {
        Serial.printf("WARNING: LVGL mutex failures: %d\n", consecutiveFailures);
      }
    }

    // Emergency restart if too many failures
    if (consecutiveFailures > 100)
    {
      Serial.println("CRITICAL: Too many UI failures - restarting");
      ESP.restart();
    }

    // Memory check
    if (millis() - systemStats.lastHeapCheck > 30000)
    {
      checkMemoryUsage();
      systemStats.lastHeapCheck = millis();
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// Network Task
void networkTask(void *parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(5000);

  Serial.println("DEBUG: Network Task started");

  for (;;)
  {
    if (ESP.getFreeHeap() < CRITICAL_HEAP)
    {
      Serial.println("CRITICAL: Low memory in network task");
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
      continue;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("DEBUG: WiFi disconnected - reconnecting");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);

      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20)
      {
        vTaskDelay(pdMS_TO_TICKS(500));
        attempts++;

        if (ESP.getFreeHeap() < CRITICAL_HEAP)
        {
          Serial.println("CRITICAL: Memory low during WiFi reconnect");
          break;
        }
      }

      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.printf("SUCCESS: WiFi reconnected - IP: %s\n", WiFi.localIP().toString().c_str());
      }
      else
      {
        Serial.println("ERROR: WiFi reconnection failed");
      }
    }

    // Handle time updates
    if (WiFi.status() == WL_CONNECTED && ESP.getFreeHeap() > MIN_FREE_HEAP)
    {
      TimeManager::update();
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// MQTT Task - Enhanced with better debugging
void mqttTask(void *parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(200);

  Serial.println("DEBUG: MQTT Task started");

  for (;;)
  {
    // Only process if WiFi connected
    if (WiFi.status() != WL_CONNECTED)
    {
      systemStats.mqttConnected = false;
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
      continue;
    }

    // Handle MQTT connection
    if (!client.connected())
    {
      systemStats.mqttConnected = false;

      if (millis() - systemStats.lastMqttReconnect > MQTT_RETRY_INTERVAL)
      {
        Serial.println("DEBUG: Attempting MQTT connection...");

        String clientId = "ESP32-2113-" + String(millis() % 10000);
        if (client.connect(clientId.c_str(), config.MQTT_USERNAME, config.MQTT_PASSWORD))
        {
          Serial.println("SUCCESS: MQTT connected");

          // Subscribe to command topic
          bool subscribed = client.subscribe(command_topic);
          Serial.printf("DEBUG: Subscription to %s: %s\n", command_topic, subscribed ? "SUCCESS" : "FAILED");

          systemStats.mqttConnected = true;

          // Send online status
          StaticJsonDocument<100> onlineDoc;
          onlineDoc["device_id"] = device_id;
          onlineDoc["status"] = "online";
          onlineDoc["timestamp"] = millis();

          char onlineBuffer[100];
          serializeJson(onlineDoc, onlineBuffer);

          bool published = client.publish(status_topic, onlineBuffer);
          Serial.printf("DEBUG: Online status published: %s\n", published ? "SUCCESS" : "FAILED");
        }
        else
        {
          Serial.printf("ERROR: MQTT connection failed, rc=%d\n", client.state());
          Serial.println("MQTT Error codes: -4=timeout, -3=connection lost, -2=connect failed, -1=disconnected, 1=bad protocol, 2=bad client id, 3=unavailable, 4=bad credentials, 5=unauthorized");
        }

        systemStats.lastMqttReconnect = millis();
      }
    }
    else
    {
      systemStats.mqttConnected = true;

      // Process MQTT messages
      client.loop();

      // Send status updates
      if (millis() - systemStats.lastStatusUpdate > STATUS_UPDATE_INTERVAL)
      {
        StaticJsonDocument<150> statusDoc;
        statusDoc["device_id"] = device_id;
        statusDoc["status"] = "running";
        statusDoc["free_heap"] = ESP.getFreeHeap();
        statusDoc["uptime"] = millis();
        statusDoc["last_message"] = systemStats.lastMqttMessage;

        char statusBuffer[150];
        serializeJson(statusDoc, statusBuffer);

        bool published = client.publish(status_topic, statusBuffer);
        Serial.printf("DEBUG: Status update published: %s\n", published ? "SUCCESS" : "FAILED");

        systemStats.lastStatusUpdate = millis();
      }
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

void setActiveArea()
{
  tft.writecommand(0x2A);
  tft.writedata(0);
  tft.writedata(0);
  tft.writedata(0);
  tft.writedata(239);

  tft.writecommand(0x2B);
  tft.writedata(0);
  tft.writedata(0);
  tft.writedata(319 >> 8);
  tft.writedata(319 & 0xFF);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== ESP32 System Starting ===");
  Serial.printf("Initial free heap: %u bytes\n", ESP.getFreeHeap());

  // Initialize topic strings
  snprintf(status_topic, sizeof(status_topic), "devices/%s/status", device_id);
  snprintf(command_topic, sizeof(command_topic), "devices/%s/commands", device_id);
  snprintf(record_topic, sizeof(record_topic), "devices/%s/record", device_id);

  Serial.printf("DEBUG: Topics initialized:\n");
  Serial.printf("  Status: %s\n", status_topic);
  Serial.printf("  Command: %s\n", command_topic);
  Serial.printf("  Record: %s\n", record_topic);

  // Validate buffer size
  if (!validateBufferSize())
  {
    Serial.println("WARNING: Using smaller buffer size");
  }

  // Initialize I2C
  Wire.begin();

  // Initialize display
  tft.init();
  setActiveArea();
  tft.setRotation(0);
  tft.setViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  tft.fillScreen(TFT_BLACK);

  Serial.printf("Free heap after display init: %u bytes\n", ESP.getFreeHeap());

  // Initialize LVGL
  lv_init();
  lv_log_register_print_cb(log_print);

  // Initialize touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(0);

  Serial.printf("Free heap after peripherals: %u bytes\n", ESP.getFreeHeap());

  // Allocate draw buffer
  draw_buf = (uint8_t *)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA);
  if (!draw_buf)
  {
    size_t fallback_size = DRAW_BUF_SIZE / 2;
    draw_buf = (uint8_t *)heap_caps_malloc(fallback_size, MALLOC_CAP_DMA);
    if (!draw_buf)
    {
      Serial.println("CRITICAL: Could not allocate draw buffer");
      while (1)
        delay(1000);
    }
    Serial.printf("WARNING: Using fallback buffer: %u bytes\n", fallback_size);
  }

  // Initialize display and input
  lv_display_t *disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf,
                                          draw_buf ? DRAW_BUF_SIZE : DRAW_BUF_SIZE / 2);
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_0);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  Serial.printf("Free heap after LVGL setup: %u bytes\n", ESP.getFreeHeap());

  // Create synchronization objects BEFORE UI init
  lvglMutex = xSemaphoreCreateMutex();
  uiUpdateQueue = xQueueCreate(5, sizeof(UIUpdate)); // Increased queue size

  if (!lvglMutex || !uiUpdateQueue)
  {
    Serial.println("CRITICAL: Failed to create sync objects");
    while (1)
      delay(1000);
  }

  Serial.println("DEBUG: Synchronization objects created");

  // Initialize UI
  ui_init();

  Serial.printf("Free heap after UI init: %u bytes\n", ESP.getFreeHeap());

  // Initialize WiFi and Time
  WiFiManager::begin(WIFI_SSID, WIFI_PASS);

  // Wait for WiFi connection
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20)
  {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.printf("\nSUCCESS: WiFi connected - IP: %s\n", WiFi.localIP().toString().c_str());
  }
  else
  {
    Serial.println("\nERROR: WiFi connection failed");
  }

  // Initialize TimeManager
  TimeManager::initialize();
  TimeManager::setIndianTime(2025, 4, 1, 19, 4, 0);

  // Configure MQTT client
  espClientSecure.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(1024);  // Increased buffer size
  client.setSocketTimeout(10); // 10 second timeout

  Serial.printf("Free heap after network init: %u bytes\n", ESP.getFreeHeap());

  // Create tasks with error checking
  BaseType_t uiResult = xTaskCreatePinnedToCore(
      uiTask, "UI_Task", UI_TASK_STACK_SIZE, NULL, 2, &uiTaskHandle, 1);

  BaseType_t netResult = xTaskCreatePinnedToCore(
      networkTask, "Net_Task", NETWORK_TASK_STACK_SIZE, NULL, 1, &networkTaskHandle, 0);

  BaseType_t mqttResult = xTaskCreatePinnedToCore(
      mqttTask, "MQTT_Task", MQTT_TASK_STACK_SIZE, NULL, 1, &mqttTaskHandle, 0);

  if (uiResult != pdPASS || netResult != pdPASS || mqttResult != pdPASS)
  {
    Serial.println("CRITICAL: Failed to create tasks");
    Serial.printf("UI: %d, Net: %d, MQTT: %d\n", uiResult, netResult, mqttResult);
    while (1)
      delay(1000);
  }

  Serial.println("SUCCESS: All tasks created");
  Serial.printf("Final free heap: %u bytes\n", ESP.getFreeHeap());

  // Test UI update after everything is initialized
  vTaskDelay(pdMS_TO_TICKS(3000));
  Serial.println("DEBUG: Testing initial UI update");
  updateUIElements("System Ready", "All systems initialized successfully");

  Serial.println("=== Setup Complete ===");
}

void loop()
{
  // Keep minimal - just watchdog feeding and diagnostics
  vTaskDelay(pdMS_TO_TICKS(5000));

  // Enhanced diagnostics every 2 minutes
  static unsigned long lastStatsUpdate = 0;
  if (millis() - lastStatsUpdate > 120000)
  {
    Serial.println("=== System Status ===");
    Serial.printf("Heap: %u bytes (min: %u)\n", ESP.getFreeHeap(), systemStats.minFreeHeap);
    Serial.printf("WiFi: %s (RSSI: %d)\n",
                  WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected",
                  WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0);
    Serial.printf("MQTT: %s (last msg: %lu ms ago)\n",
                  systemStats.mqttConnected ? "Connected" : "Disconnected",
                  systemStats.lastMqttMessage > 0 ? millis() - systemStats.lastMqttMessage : 0);
    Serial.printf("Uptime: %lu seconds\n", millis() / 1000);

    // Check task status
    if (uiTaskHandle && eTaskGetState(uiTaskHandle) == eDeleted)
    {
      Serial.println("ERROR: UI Task has been deleted!");
    }
    if (mqttTaskHandle && eTaskGetState(mqttTaskHandle) == eDeleted)
    {
      Serial.println("ERROR: MQTT Task has been deleted!");
    }
    if (networkTaskHandle && eTaskGetState(networkTaskHandle) == eDeleted)
    {
      Serial.println("ERROR: Network Task has been deleted!");
    }

    Serial.println("===================");
    lastStatsUpdate = millis();
  }
}