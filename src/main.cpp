#include <WiFi.h>
#include "ui/ui.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
// #include <Arduino.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <Wire.h> // For I2C communication
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
#define SCREEN_HEIGHT 240

// LVGL configuration
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

// Configuration
const char *WIFI_SSID = "Mukuldhy";
const char *WIFI_PASS = "12345678";

// Configuration structure
struct Config
{
  const char *MQTT_USERNAME = "mukulmqtt";
  const char *MQTT_PASSWORD = "Mukul@jaat123";
} config;

// =============== MQTT MODIFICATIONS START ===============
// Updated for single-device system with device ID 2113
const char *mqtt_server = "02ed6b84181647639b35d467c00afbd9.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char *device_id = "2113"; // Hardcoded device ID
char status_topic[50];          // Will be "devices/2113/status"
char command_topic[50];         // Will be "devices/2113/commands"
// =============== MQTT MODIFICATIONS END ===============

WiFiClientSecure espClientSecure;
PubSubClient client(espClientSecure);

SPIClass touchscreenSPI(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint8_t *draw_buf = nullptr;
uint32_t lastTick = 0;
TFT_eSPI tft = TFT_eSPI();

lv_obj_t *label = nullptr; // Added for label handling

void log_print(lv_log_level_t level, const char *buf)
{
  Serial.println(buf);
}

void update_UI()
{
  lv_timer_handler();
  lv_tick_inc(millis() - lastTick);
  lastTick = millis();
}

void touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  static TS_Point lastPoint;
  if (touchscreen.tirqTouched() && touchscreen.touched())
  {
    TS_Point p = touchscreen.getPoint();
    lastPoint = p;
    data->point.x = map(p.x, 240, 3600, 0, SCREEN_WIDTH);
    data->point.y = map(p.y, 240, 2700, 0, SCREEN_HEIGHT);
    data->state = LV_INDEV_STATE_PRESSED;
  }
  else
  {
    data->point.x = lastPoint.x;
    data->point.y = lastPoint.y;
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// =============== UPDATED CALLBACK FUNCTION ===============

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);

  // Only process messages for our command topic
  if (strcmp(topic, command_topic) != 0)
  {
    Serial.println("Ignoring message - not for our command topic");
    return;
  }

  // Check if payload is null-terminated (deserializeJson expects a string)
  if (length == 0 || payload == nullptr)
  {
    Serial.println("Error: Empty or null payload"); // Added more descriptive error
    return;
  }

  // Convert payload to null-terminated string for safe processing
  char payloadStr[length + 1];
  memcpy(payloadStr, payload, length);
  payloadStr[length] = '\0';
  Serial.print("Raw payload: "); // Added debug output
  Serial.println(payloadStr);

  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, payloadStr); // Removed unnecessary cast

  // Check for JSON parsing errors
  if (error)
  {
    Serial.print("JSON deserialize failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Updated to match Node.js server message format
  // Added default values and null checks for more robust operation
  const char *title = doc["title"] | "Reminder";     // Default if missing
  const char *description = doc["description"] | ""; // Default if missing

  // Debug output for parsed values
  Serial.print("Parsed title: ");
  Serial.println(title);
  Serial.print("Parsed description: ");
  Serial.println(description);

  // Original UI change code - kept intact
  // _ui_screen_change(&ui_screen_reminderalert, LV_SCR_LOAD_ANIM_MOVE_TOP, 500, 0, &ui_screen_homepage_screen_init);
  _ui_screen_change(&ui_screen_reminderalert, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 500, 0, &ui_screen_reminderalert_screen_init);

  // Added null checks for UI elements before updating
  if (ui_reminderalert_label_label33)
  {
    lv_label_set_text(ui_reminderalert_label_label33, description);
  }
  if (ui_reminderalert_label_label29)
  {
    lv_label_set_text(ui_reminderalert_label_label29, title);
  }

  // Original commented code kept for reference
  // if (label != nullptr)
  // {
  //   char displayText[100];
  //   snprintf(displayText, sizeof(displayText), "%s\n%s", title, description);
  //   lv_label_set_text(label, displayText);
  //   lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);
  // }

  // Send acknowledgment back to status topic
  // Improved acknowledgment with JSON serialization instead of manual formatting
  DynamicJsonDocument ackDoc(200);
  ackDoc["device_id"] = device_id;
  ackDoc["status"] = "received";
  ackDoc["reminder"] = title;
  ackDoc["timestamp"] = millis(); // Added timestamp for tracking

  char ackMsg[256];
  serializeJson(ackDoc, ackMsg);

  if (!client.publish(status_topic, ackMsg))
  {
    Serial.println("Failed to publish acknowledgment"); // Added error handling
  }
  // Original acknowledgment code kept for reference:
  // char ackMsg[100];
  // snprintf(ackMsg, sizeof(ackMsg),
  //          "{\"device_id\":\"%s\",\"status\":\"received\",\"reminder\":\"%s\"}",
  //          device_id, title);
  // client.publish(status_topic, ackMsg);
}

// Callback for the Sned Alert Page

void ui_event_sendalertpage_image_image9(lv_event_t *e)
{
  Serial.println("Sending alerts to the caregiver");
  DynamicJsonDocument ackDoc(200);
  ackDoc["device_id"] = device_id;
  ackDoc["status"] = "received";
  ackDoc["reminder"] = "Alert";
  ackDoc["timestamp"] = millis(); // Added timestamp for tracking

  char ackMsg[256];
  serializeJson(ackDoc, ackMsg);

  if (!client.publish(status_topic, ackMsg))
  {
    Serial.println("Failed to publish acknowledgment"); // Added error handling
  }
}

// =============== END OF UPDATED CALLBACK ===============

// =============== UPDATED RECONNECT FUNCTION ===============
void reconnect()
{
  IPAddress mqtt_ip;
  if (!WiFi.hostByName(mqtt_server, mqtt_ip))
  {
    Serial.println("DNS lookup failed. Retrying...");
    delay(5000);
    return;
  }
  Serial.print("Resolved MQTT broker IP: ");
  Serial.println(mqtt_ip);

  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection as device ");
    Serial.println(device_id);

    // Use device ID as client ID
    String clientId = "ESP32-" + String(device_id);

    if (client.connect(clientId.c_str(), config.MQTT_USERNAME, config.MQTT_PASSWORD))
    {
      Serial.println("Connected to MQTT broker");
      client.subscribe(command_topic);
      Serial.print("Subscribed to: ");
      Serial.println(command_topic);

      // Send initial status message
      char initMsg[100];
      snprintf(initMsg, sizeof(initMsg),
               "{\"device_id\":\"%s\",\"status\":\"online\",\"ip\":\"%s\"}",
               device_id, WiFi.localIP().toString().c_str());
      client.publish(status_topic, initMsg);
    }
    else
    {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}
// =============== END OF UPDATED RECONNECT ===============

void setActiveArea()
{
  // Column address range (0-239)
  tft.writecommand(0x2A); // CASET command
  tft.writedata(0);
  tft.writedata(0);
  tft.writedata(0);
  tft.writedata(239);

  // Row address range (40-279 for centered 240x240)
  tft.writecommand(0x2B); // PASET command
  tft.writedata(40 >> 8);
  tft.writedata(40 & 0xFF); // Start row = 40
  tft.writedata(279 >> 8);
  tft.writedata(279 & 0xFF); // End row = 279
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  // =============== MQTT TOPIC INITIALIZATION ===============
  // Initialize topics for single-device system
  sprintf(status_topic, "devices/%s/status", device_id);
  sprintf(command_topic, "devices/%s/commands", device_id);
  // =============== END TOPIC INIT ===============

  // Initialize I2C bus first (for RTC)
  Wire.begin();

  // Initialize display
  tft.init();
  setActiveArea();
  tft.setRotation(1);
  tft.setViewport(0, 40, 240, 240);
  tft.fillScreen(TFT_BLACK);

  // Initialize LVGL
  lv_init();
  lv_log_register_print_cb(log_print);

  // Initialize touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(0);

  // Allocate draw buffer
  draw_buf = (uint8_t *)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA);
  if (!draw_buf)
  {
    Serial.println("Failed to allocate draw buffer");
    while (1)
      ;
  }

  // Initialize display
  lv_display_t *disp = lv_tft_espi_create(SCREEN_HEIGHT, SCREEN_WIDTH, draw_buf, DRAW_BUF_SIZE);
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_0);

  // Initialize input device
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  // Initialize UI
  ui_init();

  delay(2000);
  // Initialize WiFi
  WiFiManager::begin(WIFI_SSID, WIFI_PASS);

  // Initialize Time
  TimeManager::initialize();
  TimeManager::setIndianTime(2025, 4, 1, 19, 4, 0); // 25th Dec 2023, 3:30 PM IST

  // Configure WiFiClientSecure
  espClientSecure.setInsecure(); // Bypass certificate verification (for testing)

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop()
{
  update_UI();
  WiFiManager::handle();

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected!");
    delay(1000);
    return;
  }

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  TimeManager::update();

  // Periodically send device status
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 30000)
  { // Every 30 seconds
    char statusMsg[150];
    snprintf(statusMsg, sizeof(statusMsg),
             "{\"device_id\":\"%s\",\"status\":\"online\",\"time\":\"%s\",\"rssi\":%d,\"free_heap\":%u}",
             device_id,
             TimeManager::getFormattedTime().c_str(),
             WiFi.RSSI(),
             ESP.getFreeHeap());
    client.publish(status_topic, statusMsg);
    lastStatusUpdate = millis();
  }

  delay(2);
}