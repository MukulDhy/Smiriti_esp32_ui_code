#include <WiFi.h>
#include "ui/ui.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <Wire.h> // For I2C communication
#include "tasks/wifi.h"
#include "tasks/time.h"

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

SPIClass touchscreenSPI(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint8_t *draw_buf = nullptr;
uint32_t lastTick = 0;
TFT_eSPI tft = TFT_eSPI();

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

  // Call this once at startup
  TimeManager::initialize();

  // To set specific Indian time (optional)
  TimeManager::setIndianTime(2025, 4, 1, 19, 4, 0); // 25th Dec 2023, 3:30 PM IST

  // In setup():
  // TimeManager::begin();
}

void loop()
{
  update_UI();
  WiFiManager::handle();

  // // Sync time with NTP when WiFi is connected
  // static bool timeSynced = false;
  // if (WiFiManager::getState() == WiFiManager::State::CONNECTED && !timeSynced)
  // {
  //   timeSynced = TimeManager::syncWithNTP();
  // }
  // TimeManager::update();

  // Call this in your main loop
  TimeManager::update();

  delay(2);
}