#include "ui/ui.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

//----------------------------------------

// Defines the T_CS Touchscreen PIN.
// Touchscreen pins
#define XPT2046_IRQ 36  // T_IRQ
#define XPT2046_MOSI 32 // T_DIN
#define XPT2046_MISO 39 // T_OUT
#define XPT2046_CLK 25  // T_CLK
#define XPT2046_CS 33   // T_CS
#define T_CS_PIN 13     //--> T_CS

// Defines the Touchscreen calibration result value.
// Replace with the calibration results on your TFT LCD Touchscreen.
#define touchscreen_Min_X 203
#define touchscreen_Max_X 3700
#define touchscreen_Min_Y 240
#define touchscreen_Max_Y 3721
//----------------------------------------

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Defines the screen resolution.
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

// LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes.
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint8_t *draw_buf; // Changed from array of pointers to simple pointer

// Variables for x, y and z values ​​on the touchscreen.
uint16_t x, y, z;

// Used to track the tick timer.
uint32_t lastTick = 0;

unsigned long previousMillis = 0;
const long interval = 2000;

void log_print(lv_log_level_t level, const char *buf)
{
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

void update_UI()
{
  lv_tick_inc(millis() - lastTick);
  lastTick = millis();
  lv_timer_handler();
}

void touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  if (touchscreen.tirqTouched() && touchscreen.touched())
  {
    TS_Point p = touchscreen.getPoint();
    x = map(p.x, touchscreen_Max_X, touchscreen_Min_X, 1, SCREEN_WIDTH);
    y = map(p.y, touchscreen_Max_Y, touchscreen_Min_Y, 1, SCREEN_HEIGHT);
    z = p.z;

    data->point.x = x;
    data->point.y = y;
    data->state = LV_INDEV_STATE_PRESSED;
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  delay(2000);

  Serial.println("ESP32 + TFT LCD Touchscreen ILI9341 240x320 + LVGL + EEZ Studio");
  Serial.println("Simple Multi-Screen");
  Serial.println();
  delay(500);

  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);
  delay(500);

  touchscreen.begin();
  touchscreen.setRotation(2);

  //---------------------------------------- LVGL setup.
  Serial.println();
  Serial.println("Start LVGL Setup.");
  delay(500);

  lv_init();
  lv_log_register_print_cb(log_print);

  // Allocate the draw buffer
  draw_buf = (uint8_t *)heap_caps_malloc(DRAW_BUF_SIZE * sizeof(uint8_t), MALLOC_CAP_DMA);
  if (draw_buf == nullptr)
  {
    Serial.println("Failed to allocate draw buffer!");
    while (1)
      ;
  }

  lv_display_t *disp = lv_tft_espi_create(SCREEN_HEIGHT, SCREEN_WIDTH, draw_buf, DRAW_BUF_SIZE);
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  Serial.println();
  Serial.println("LVGL Setup Completed.");
  delay(500);
  //----------------------------------------

  ui_init();
  delay(500);
}

void loop()
{
  update_UI();
  delay(5);
}