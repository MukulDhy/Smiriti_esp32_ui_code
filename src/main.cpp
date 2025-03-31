// main.cpp
#include <WiFi.h>
#include "ui/ui.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <tasks/wifi.h>

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

SPIClass touchscreenSPI(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint8_t *draw_buf = nullptr;
uint32_t lastTick = 0;

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

TFT_eSPI tft = TFT_eSPI();
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
    delay(100); // Reduced initial delay

    // Additional Code of the TFT - to set the 240 * 240
    tft.init();
    setActiveArea();
    tft.setRotation(1); // Adjust rotation if needed
    // Define the 240x240 viewport (centered vertically)
    tft.setViewport(0, 40, 240, 240); // (x, y, width, height)
    tft.fillScreen(TFT_BLACK);        // Test: fill only the viewport area

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
        while (1)
            ;

    // Initialize display
    lv_display_t *disp = lv_tft_espi_create(SCREEN_HEIGHT, SCREEN_WIDTH, draw_buf, DRAW_BUF_SIZE);
    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_0);

    // Initialize input device
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchscreen_read);

    ui_init();
    initWiFi(); // Modified WiFi initialization
}

void loop()
{
    update_UI();
    handleWiFi(); // Non-blocking WiFi handling
    delay(2); // Reduced delay for better responsiveness
}