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
  if (xSemaphoreTake(lvglMutex, pdMS_TO_TICKS(100)) == pdTRUE)
  {
    // Check if we're not in the middle of rendering
    lv_display_t *disp = lv_display_get_default();
    if (disp && !disp->rendering_in_progress)
    {
      _ui_screen_change(&ui_screen_reminderalert, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 500, 0, &ui_screen_reminderalert_screen_init);

      if (ui_reminderalert_label_label33)
      {
        lv_label_set_text(ui_reminderalert_label_label33, description);
      }
      if (ui_reminderalert_label_label29)
      {
        lv_label_set_text(ui_reminderalert_label_label29, title);
      }
    }
    xSemaphoreGive(lvglMutex);
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
void callback(char *topic, byte *payload, unsigned int length)
{
  // Basic validation only
  if (!topic || !payload || length == 0 || length > 200)
  {
    return;
  }

  // Only process our command topic
  if (strcmp(topic, command_topic) != 0)
  {
    return;
  }

  // Create simple string from payload
  char payloadStr[201];
  size_t copyLen = min(length, (unsigned int)200);
  memcpy(payloadStr, payload, copyLen);
  payloadStr[copyLen] = '\0';

  // Simple JSON parsing - AVOID HEAVY OPERATIONS
  char *titleStart = strstr(payloadStr, "\"title\":\"");
  char *descStart = strstr(payloadStr, "\"description\":\"");

  UIUpdate uiUpdate = {"Reminder", "", false};

  if (titleStart)
  {
    titleStart += 9; // Skip "title":"
    char *titleEnd = strchr(titleStart, '"');
    if (titleEnd && (titleEnd - titleStart) < 31)
    {
      strncpy(uiUpdate.title, titleStart, titleEnd - titleStart);
      uiUpdate.title[titleEnd - titleStart] = '\0';
    }
  }

  if (descStart)
  {
    descStart += 14; // Skip "description":"
    char *descEnd = strchr(descStart, '"');
    if (descEnd && (descEnd - descStart) < 63)
    {
      strncpy(uiUpdate.description, descStart, descEnd - descStart);
      uiUpdate.description[descEnd - descStart] = '\0';
    }
  }

  // Send to UI queue (non-blocking)
  xQueueSend(uiUpdateQueue, &uiUpdate, 0);
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
  const TickType_t xFrequency = pdMS_TO_TICKS(10); // Reduced to 10ms for stability

  UIUpdate uiUpdate;

  for (;;)
  {
    // Handle UI updates from queue
    while (xQueueReceive(uiUpdateQueue, &uiUpdate, 0) == pdTRUE)
    {
      updateUIElements(uiUpdate.title, uiUpdate.description);
      vTaskDelay(pdMS_TO_TICKS(10)); // Small delay after UI update
    }

    // Update LVGL - THREAD SAFE
    if (xSemaphoreTake(lvglMutex, pdMS_TO_TICKS(5)) == pdTRUE)
    {
      uint32_t currentTick = millis();
      lv_tick_inc(currentTick - lastTick);
      lastTick = currentTick;

      lv_timer_handler();
      xSemaphoreGive(lvglMutex);
    }

    // Memory check every 10 seconds
    if (millis() - systemStats.lastHeapCheck > 10000)
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
  const TickType_t xFrequency = pdMS_TO_TICKS(1000); // Reduced frequency - 1 second

  for (;;)
  {
    // Simple WiFi check
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi disconnected - reconnecting");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);

      // Simple wait
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 30)
      {
        vTaskDelay(pdMS_TO_TICKS(500));
        attempts++;
      }

      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println("WiFi reconnected");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
      }
    }

    // Handle time updates
    // TimeManager::handle();
    TimeManager::update();

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
  delay(500); // Increased initial delay

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
    // Don't halt - continue with available memory
  }

  // Initialize I2C
  Wire.begin();

  // Initialize display
  tft.init();
  setActiveArea();
  tft.setRotation(0);
  tft.setViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  tft.fillScreen(TFT_BLACK);

  // Initialize LVGL
  lv_init();
  lv_log_register_print_cb(log_print);

  // Initialize touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(0);

  // Allocate draw buffer with fallback
  draw_buf = (uint8_t *)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA);
  if (!draw_buf)
  {
    // Try smaller buffer
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
  else
  {
    Serial.printf("Draw buffer allocated: %u bytes\n", DRAW_BUF_SIZE);
  }

  // Initialize display and input
  lv_display_t *disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf,
                                          draw_buf ? DRAW_BUF_SIZE : DRAW_BUF_SIZE / 2);
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_0);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  // Initialize UI
  ui_init();

  // Initialize WiFi and Time
  WiFiManager::begin(WIFI_SSID, WIFI_PASS);
  TimeManager::initialize();
  TimeManager::setIndianTime(2025, 4, 1, 19, 4, 0);

  // Configure MQTT client
  espClientSecure.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(512); // Limit buffer size

  // Create synchronization objects - SIMPLIFIED
  lvglMutex = xSemaphoreCreateMutex();
  uiUpdateQueue = xQueueCreate(2, sizeof(UIUpdate)); // Smaller queue

  if (!lvglMutex || !uiUpdateQueue)
  {
    Serial.println("Failed to create sync objects");
    while (1)
      delay(1000);
  }

  Serial.printf("Free heap after init: %u bytes\n", ESP.getFreeHeap());

  // Create tasks with error checking
  BaseType_t uiResult = xTaskCreatePinnedToCore(
      uiTask, "UI_Task", UI_TASK_STACK_SIZE, NULL, 2, &uiTaskHandle, 1);

  BaseType_t netResult = xTaskCreatePinnedToCore(
      networkTask, "Net_Task", NETWORK_TASK_STACK_SIZE, NULL, 1, &networkTaskHandle, 0);

  BaseType_t mqttResult = xTaskCreatePinnedToCore(
      mqttTask, "MQTT_Task", MQTT_TASK_STACK_SIZE, NULL, 1, &mqttTaskHandle, 0);

  if (uiResult != pdPASS || netResult != pdPASS || mqttResult != pdPASS)
  {
    Serial.println("Failed to create tasks - insufficient memory");
    Serial.printf("UI: %d, Net: %d, MQTT: %d\n", uiResult, netResult, mqttResult);
    while (1)
      delay(1000);
  }

  Serial.println("All tasks created successfully");
  Serial.printf("Final free heap: %u bytes\n", ESP.getFreeHeap());
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