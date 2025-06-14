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

// Task stack sizes (INCREASED - this was the main issue)
#define UI_TASK_STACK_SIZE 12288     // Increased from 8192
#define NETWORK_TASK_STACK_SIZE 8192 // Increased from 4096
#define MQTT_TASK_STACK_SIZE 8192    // Increased from 3072 - CRITICAL FIX

// LVGL configuration with memory optimization
#define DRAW_BUF_MULTIPLIER 10 // Back to 10 for stability
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / DRAW_BUF_MULTIPLIER * (LV_COLOR_DEPTH / 8))

// Configuration
const char *WIFI_SSID = "Mukuldhy";
const char *WIFI_PASS = "12345678";

// Configuration structure
struct Config
{
  const char *MQTT_USERNAME = "mukulmqtt";
  const char *MQTT_PASSWORD = "Mukul@jaat123";
} config;

// MQTT Configuration - optimized with connection management
const char *mqtt_server = "02ed6b84181647639b35d467c00afbd9.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char *device_id = "2113";
char status_topic[50];
char command_topic[50];
char record_topic[50];

// Connection timeouts and retry intervals
#define WIFI_TIMEOUT_MS 15000
#define MQTT_TIMEOUT_MS 10000
#define MQTT_RETRY_INTERVAL 10000 // Increased retry interval
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

// Synchronization - SIMPLIFIED
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
};
SystemStats systemStats;

// SIMPLIFIED message structure
struct UIUpdate
{
  char title[32];       // Reduced size
  char description[64]; // Reduced size
  bool isAlert;
};

// Optimized calibration values
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

// Logging with memory awareness
void log_print(lv_log_level_t level, const char *buf)
{
  if (ESP.getFreeHeap() > CRITICAL_HEAP)
  {
    Serial.println(buf);
  }
}

// THREAD-SAFE UI update function

void updateUIElements(const char *title, const char *description)
{
  if (!title || !description || !lvglMutex)
  {
    return;
  }

  // Additional validation
  if (strlen(title) > 100 || strlen(description) > 200)
  {
    Serial.println("UI update strings too long");
    return;
  }

  // Try to take mutex with timeout
  if (xSemaphoreTake(lvglMutex, pdMS_TO_TICKS(50)) == pdTRUE)
  {
    // CRITICAL: Check rendering state multiple times
    lv_display_t *disp = lv_display_get_default();

    // Wait for any ongoing rendering to complete
    int renderWaitCount = 0;
    while (disp && disp->rendering_in_progress && renderWaitCount < 10)
    {
      xSemaphoreGive(lvglMutex);
      vTaskDelay(pdMS_TO_TICKS(5)); // Wait 5ms
      renderWaitCount++;

      if (xSemaphoreTake(lvglMutex, pdMS_TO_TICKS(50)) != pdTRUE)
      {
        Serial.println("Failed to retake mutex after render wait");
        return;
      }
      disp = lv_display_get_default();
    }

    // If still rendering after wait, skip this update
    if (disp && disp->rendering_in_progress)
    {
      Serial.println("Still rendering - skipping UI update");
      xSemaphoreGive(lvglMutex);
      return;
    }

    // Check memory before proceeding
    if (ESP.getFreeHeap() > MIN_FREE_HEAP && disp)
    {
      // Safely check current screen
      lv_obj_t *current_screen = lv_scr_act();

      // Only change screen if not already on reminder alert
      if (current_screen && current_screen != ui_screen_reminderalert)
      {
        // Use fade animation instead of move for less processing
        _ui_screen_change(&ui_screen_reminderalert, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, &ui_screen_reminderalert_screen_init);

        // Give time for screen change
        xSemaphoreGive(lvglMutex);
        vTaskDelay(pdMS_TO_TICKS(250)); // Wait for screen change

        // Retake mutex for label updates
        if (xSemaphoreTake(lvglMutex, pdMS_TO_TICKS(50)) != pdTRUE)
        {
          Serial.println("Failed to retake mutex for label update");
          return;
        }
      }

      // Update labels with additional validation
      if (ui_reminderalert_label_label33 && lv_obj_is_valid(ui_reminderalert_label_label33))
      {
        // Check if object is on current screen
        lv_obj_t *parent = lv_obj_get_screen(ui_reminderalert_label_label33);
        if (parent == lv_scr_act())
        {
          lv_label_set_text(ui_reminderalert_label_label33, description);
        }
      }

      if (ui_reminderalert_label_label29 && lv_obj_is_valid(ui_reminderalert_label_label29))
      {
        // Check if object is on current screen
        lv_obj_t *parent = lv_obj_get_screen(ui_reminderalert_label_label29);
        if (parent == lv_scr_act())
        {
          lv_label_set_text(ui_reminderalert_label_label29, title);
        }
      }
    }
    else
    {
      Serial.println("Skipping UI update - low memory or invalid display");
    }

    xSemaphoreGive(lvglMutex);
  }
  else
  {
    Serial.println("Failed to take mutex for UI update");
  }
}
// Optimized touchscreen handling
void touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  static TS_Point lastValidPoint = {0, 0, 0};
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();

  // Debounce touch readings
  if (currentTime - lastReadTime < 20)
  { // Increased debounce
    data->point.x = lastValidPoint.x;
    data->point.y = lastValidPoint.y;
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  if (touchscreen.tirqTouched() && touchscreen.touched())
  {
    TS_Point p = touchscreen.getPoint();

    // Validate touch coordinates
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

// SIMPLIFIED MQTT callback - NO STACK-HEAVY OPERATIONS
// 4. Add this function to check LVGL state
bool isLVGLSafeToUpdate()
{
  lv_display_t *disp = lv_display_get_default();
  if (!disp)
  {
    return false;
  }

  return !disp->rendering_in_progress;
}

// 5. Enhanced callback function to prevent rendering conflicts
void callback(char *topic, byte *payload, unsigned int length)
{
  // Basic validation
  if (!topic || !payload || length == 0 || length > 200)
  {
    return;
  }

  // Only process our command topic
  if (strcmp(topic, command_topic) != 0)
  {
    return;
  }

  // Check if LVGL is safe to update before queuing
  if (!isLVGLSafeToUpdate())
  {
    Serial.println("LVGL not safe - skipping callback");
    return;
  }

  // Create simple string from payload
  char payloadStr[201];
  size_t copyLen = min(length, (unsigned int)200);
  memcpy(payloadStr, payload, copyLen);
  payloadStr[copyLen] = '\0';

  // Simple JSON parsing
  char *titleStart = strstr(payloadStr, "\"title\":\"");
  char *descStart = strstr(payloadStr, "\"description\":\"");

  UIUpdate uiUpdate = {"Reminder", "", false};

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

  // Send to UI queue (non-blocking)
  if (xQueueSend(uiUpdateQueue, &uiUpdate, 0) != pdTRUE)
  {
    Serial.println("UI queue full - dropping update");
  }
}

// Event handlers - SIMPLIFIED
void callAlertToBackend(lv_event_t *e)
{
  if (systemStats.mqttConnected)
  {
    const char *alertMsg = "{\"device_id\":\"2113\",\"status\":\"alert\",\"type\":\"emergency\"}";
    client.publish(status_topic, alertMsg);
  }
}

void sendRecordingRequestToBackend(lv_event_t *e)
{
  if (systemStats.mqttConnected)
  {
    const char *recordMsg = "{\"device_id\":\"2113\",\"status\":\"record_request\"}";
    client.publish(record_topic, recordMsg);
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
    Serial.println("Critical memory - restarting");
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP.restart();
  }
}

bool validateBufferSize()
{
  size_t requiredSize = DRAW_BUF_SIZE;
  size_t availableHeap = ESP.getMaxAllocHeap();

  if (requiredSize > availableHeap * 0.2)
  { // More conservative - 20%
    Serial.printf("Buffer too large: %u > %u\n", requiredSize, (uint32_t)(availableHeap * 0.2));
    return false;
  }
  return true;
}

// UI Task - Core 1 - THREAD SAFE LVGL
void uiTask(void *parameter)
{
  uint32_t lastTick = millis();
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(25); // Slightly increased

  UIUpdate uiUpdate;
  int consecutiveFailures = 0;
  bool renderingWarningShown = false;

  for (;;)
  {
    // Check memory before processing
    if (ESP.getFreeHeap() < CRITICAL_HEAP)
    {
      Serial.println("Critical memory in UI task");
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    // Handle UI updates from queue with limits
    int updateCount = 0;
    while (xQueueReceive(uiUpdateQueue, &uiUpdate, 0) == pdTRUE && updateCount < 2)
    {
      updateUIElements(uiUpdate.title, uiUpdate.description);
      vTaskDelay(pdMS_TO_TICKS(100)); // Longer delay after UI update
      updateCount++;
    }

    // Update LVGL with enhanced protection
    if (xSemaphoreTake(lvglMutex, pdMS_TO_TICKS(30)) == pdTRUE) // Increased timeout
    {
      // Check if rendering is safe
      lv_display_t *disp = lv_display_get_default();

      if (!disp)
      {
        Serial.println("Display not available");
        xSemaphoreGive(lvglMutex);
        consecutiveFailures++;
      }
      else if (disp->rendering_in_progress)
      {
        // Just skip this iteration if rendering
        if (!renderingWarningShown)
        {
          Serial.println("Skipping LVGL update - rendering in progress");
          renderingWarningShown = true;
        }
        xSemaphoreGive(lvglMutex);
      }
      else
      {
        // Safe to update LVGL
        renderingWarningShown = false;

        uint32_t currentTick = millis();
        uint32_t elapsed = currentTick - lastTick;

        // Limit tick increment to prevent overflow
        if (elapsed > 0 && elapsed < 1000)
        {
          lv_tick_inc(elapsed);
        }
        lastTick = currentTick;

        // Call timer handler
        lv_timer_handler();
        consecutiveFailures = 0;

        xSemaphoreGive(lvglMutex);
      }
    }
    else
    {
      consecutiveFailures++;
      if (consecutiveFailures % 10 == 0) // Log every 10th failure
      {
        Serial.printf("Failed to take LVGL mutex, failures: %d\n", consecutiveFailures);
      }
    }

    // Emergency restart if too many failures
    if (consecutiveFailures > 50)
    {
      Serial.println("Too many UI failures - restarting");
      ESP.restart();
    }

    // Memory check every 30 seconds
    if (millis() - systemStats.lastHeapCheck > 30000)
    {
      checkMemoryUsage();
      systemStats.lastHeapCheck = millis();
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// Network Task - Core 0 - SIMPLIFIED

void networkTask(void *parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(5000); // Increased to 5 seconds

  for (;;)
  {
    // Simple WiFi check with memory protection
    if (ESP.getFreeHeap() < CRITICAL_HEAP)
    {
      Serial.println("Critical memory in network task");
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
      continue;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi disconnected - reconnecting");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);

      // Simple wait with timeout
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) // Reduced attempts
      {
        vTaskDelay(pdMS_TO_TICKS(500));
        attempts++;

        // Check memory during connection attempts
        if (ESP.getFreeHeap() < CRITICAL_HEAP)
        {
          Serial.println("Memory critical during WiFi reconnect");
          break;
        }
      }

      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println("WiFi reconnected");
      }
    }

    // Handle time updates ONLY if WiFi is connected and memory is OK
    if (WiFi.status() == WL_CONNECTED && ESP.getFreeHeap() > MIN_FREE_HEAP)
    {
      TimeManager::update();
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// MQTT Task - Core 0 - HEAVILY OPTIMIZED TO PREVENT STACK OVERFLOW
void mqttTask(void *parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(200); // Reduced frequency - 200ms

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
        Serial.println("Attempting MQTT connection...");

        // Simple connection attempt - NO DNS RESOLUTION
        String clientId = "ESP32-2113-" + String(millis() % 1000);
        if (client.connect(clientId.c_str(), config.MQTT_USERNAME, config.MQTT_PASSWORD))
        {
          Serial.println("MQTT connected");
          client.subscribe(command_topic);
          systemStats.mqttConnected = true;

          // Simple status message
          client.publish(status_topic, "{\"device_id\":\"2113\",\"status\":\"online\"}");
        }
        else
        {
          Serial.printf("MQTT failed, rc=%d\n", client.state());
        }

        systemStats.lastMqttReconnect = millis();
      }
    }
    else
    {
      systemStats.mqttConnected = true;

      // Process MQTT - this should be lightweight
      client.loop();

      // Send status updates
      if (millis() - systemStats.lastStatusUpdate > STATUS_UPDATE_INTERVAL)
      {
        // Simple status message to avoid sprintf stack usage
        client.publish(status_topic, "{\"device_id\":\"2113\",\"status\":\"running\"}");
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
  delay(1000); // Increased initial delay

  Serial.println("Starting ESP32 System...");
  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());

  // Initialize topic strings
  snprintf(status_topic, sizeof(status_topic), "devices/%s/status", device_id);
  snprintf(command_topic, sizeof(command_topic), "devices/%s/commands", device_id);
  snprintf(record_topic, sizeof(record_topic), "devices/%s/record", device_id);

  // Validate buffer size
  if (!validateBufferSize())
  {
    Serial.println("Invalid buffer size - using smaller buffer");
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

  // Allocate draw buffer with fallback
  draw_buf = (uint8_t *)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA);
  if (!draw_buf)
  {
    size_t fallback_size = DRAW_BUF_SIZE / 2;
    draw_buf = (uint8_t *)heap_caps_malloc(fallback_size, MALLOC_CAP_DMA);
    if (!draw_buf)
    {
      Serial.println("Could not allocate any draw buffer");
      while (1)
        delay(1000);
    }
    Serial.printf("Using fallback buffer: %u bytes\n", fallback_size);
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
  uiUpdateQueue = xQueueCreate(2, sizeof(UIUpdate));

  if (!lvglMutex || !uiUpdateQueue)
  {
    Serial.println("Failed to create sync objects");
    while (1)
      delay(1000);
  }

  // Initialize UI - AFTER sync objects are created
  ui_init();

  Serial.printf("Free heap after UI init: %u bytes\n", ESP.getFreeHeap());

  // Initialize WiFi and Time
  WiFiManager::begin(WIFI_SSID, WIFI_PASS);

  // CRITICAL: Initialize TimeManager AFTER UI is ready
  TimeManager::initialize();
  TimeManager::setIndianTime(2025, 4, 1, 19, 4, 0);

  // Configure MQTT client
  espClientSecure.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(512);

  Serial.printf("Free heap after network init: %u bytes\n", ESP.getFreeHeap());

  // Create tasks with error checking - REDUCED STACK SIZES
  BaseType_t uiResult = xTaskCreatePinnedToCore(
      uiTask, "UI_Task", 10240, NULL, 2, &uiTaskHandle, 1); // Reduced

  BaseType_t netResult = xTaskCreatePinnedToCore(
      networkTask, "Net_Task", 6144, NULL, 1, &networkTaskHandle, 0); // Reduced

  BaseType_t mqttResult = xTaskCreatePinnedToCore(
      mqttTask, "MQTT_Task", 6144, NULL, 1, &mqttTaskHandle, 0); // Reduced

  if (uiResult != pdPASS || netResult != pdPASS || mqttResult != pdPASS)
  {
    Serial.println("Failed to create tasks - insufficient memory");
    Serial.printf("UI: %d, Net: %d, MQTT: %d\n", uiResult, netResult, mqttResult);
    while (1)
      delay(1000);
  }

  Serial.println("All tasks created successfully");
  Serial.printf("Final free heap: %u bytes\n", ESP.getFreeHeap());

  // Wait for system to stabilize
  vTaskDelay(pdMS_TO_TICKS(2000));
}
void loop()
{
  // Keep minimal - just watchdog feeding
  vTaskDelay(pdMS_TO_TICKS(5000)); // 5 second delay

  // Simple stats every 2 minutes
  static unsigned long lastStatsUpdate = 0;
  if (millis() - lastStatsUpdate > 120000)
  {
    Serial.printf("Heap: %u, Min: %u, WiFi: %s, MQTT: %s\n",
                  ESP.getFreeHeap(), systemStats.minFreeHeap,
                  WiFi.status() == WL_CONNECTED ? "OK" : "FAIL",
                  systemStats.mqttConnected ? "OK" : "FAIL");
    lastStatsUpdate = millis();
  }
}