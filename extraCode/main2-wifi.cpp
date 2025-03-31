#include <Arduino.h>
#include <WiFi.h>
#include "lvgl.h"
#include "ui.h"

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Screen management variables
bool wifiConnected = false;
unsigned long lastWifiCheck = 0;
const unsigned long wifiCheckInterval = 5000; // Check every 5 seconds

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[DISPLAY_BUFFER_SIZE];

// Function declarations
void checkWiFiConnection();
void updateScreenBasedOnWiFiStatus();
void connectToWiFi();
void wifiEventCallback(WiFiEvent_t event, WiFiEventInfo_t info);

void setup() {
  Serial.begin(115200);
  
  // Initialize LVGL
  lv_init();
  
  // Initialize display
  tft.begin();
  tft.setRotation(0);
  
  // Initialize LVGL display buffer
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, DISPLAY_BUFFER_SIZE);
  
  // Initialize display driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
  
  // Initialize touch driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
  
  // Create UI
  ui_init();
  
  // Set initial screen to connection screen
  lv_scr_load(ui_ConnectionScreen);
  
  // Register WiFi event handler
  WiFi.onEvent(wifiEventCallback);
  
  // Attempt initial connection
  connectToWiFi();
}

void loop() {
  lv_timer_handler(); // Handle LVGL tasks
  
  // Periodically check WiFi connection
  if (millis() - lastWifiCheck > wifiCheckInterval) {
    checkWiFiConnection();
    lastWifiCheck = millis();
  }
  
  delay(5);
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  lv_label_set_text(ui_ConnectionStatusLabel, "Connecting to WiFi...");
  
  WiFi.begin(ssid, password);
  
  // Set timeout for connection attempt (15 seconds)
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    wifiConnected = true;
    lv_label_set_text(ui_ConnectionStatusLabel, "Connected!");
    updateScreenBasedOnWiFiStatus();
  } else {
    Serial.println("\nFailed to connect");
    wifiConnected = false;
    lv_label_set_text(ui_ConnectionStatusLabel, "Connection failed. Retrying...");
    updateScreenBasedOnWiFiStatus();
  }
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      // We were connected but now we're not
      Serial.println("WiFi disconnected!");
      wifiConnected = false;
      updateScreenBasedOnWiFiStatus();
    }
    // Attempt reconnection
    connectToWiFi();
  } else if (!wifiConnected) {
    // We weren't connected but now we are
    Serial.println("WiFi reconnected!");
    wifiConnected = true;
    updateScreenBasedOnWiFiStatus();
  }
}

void updateScreenBasedOnWiFiStatus() {
  if (wifiConnected) {
    lv_scr_load(ui_MainScreen);
    lv_label_set_text(ui_WifiStatusLabel, "Connected");
    lv_obj_set_style_text_color(ui_WifiStatusLabel, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
  } else {
    lv_scr_load(ui_ConnectionScreen);
    lv_label_set_text(ui_WifiStatusLabel, "Disconnected");
    lv_obj_set_style_text_color(ui_WifiStatusLabel, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

void wifiEventCallback(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("Connected to access point");
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("Disconnected from WiFi access point");
      wifiConnected = false;
      updateScreenBasedOnWiFiStatus();
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.print("Got IP: ");
      Serial.println(WiFi.localIP());
      wifiConnected = true;
      updateScreenBasedOnWiFiStatus();
      break;
    default:
      break;
  }
}