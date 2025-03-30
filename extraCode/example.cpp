#include <WiFi.h>
#include <HTTPClient.h>
#include <lvgl.h>
#include <TFT_eSPI.h> // Replace with your display library

/********************* LVGL + Display Setup *********************/
#define LVGL_TICK_PERIOD 5 // Milliseconds

TFT_eSPI tft; // Replace with your display driver
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[TFT_WIDTH * 10]; // Adjust buffer size

// Display flushing function
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors(&color_p->full, w * h, true);
  tft.endWrite();
  lv_disp_flush_ready(disp);
}

// LVGL setup
void setup_lvgl() {
  lv_init();
  tft.begin(); // Initialize display
  tft.setRotation(1);
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 10);
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
}

/********************* WiFi Setup *********************/
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

void connect_wifi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
}

/********************* UI Elements *********************/
lv_obj_t *spinner;    // Loading spinner
lv_obj_t *reminder_label; // Label to display reminder data

void show_loading(bool show) {
  if (show) {
    lv_obj_clear_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(reminder_label, LV_OBJ_FLAG_HIDDEN); // Hide reminder text
  } else {
    lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(reminder_label, LV_OBJ_FLAG_HIDDEN); // Show reminder text
  }
}

/********************* Fetch Reminder Data *********************/
void fetch_reminder_data() {
  show_loading(true); // Show spinner, hide label
  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://your-api.com/reminders/1"); // Replace with your API
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Reminder Data: " + payload);
      
      // Parse JSON (example: {"title": "Buy milk", "time": "18:00"})
      // Use ArduinoJson if needed. Here, we display raw payload for simplicity.
      lv_label_set_text(reminder_label, payload.c_str());
    } else {
      lv_label_set_text(reminder_label, "Failed to fetch reminder!");
      Serial.println("HTTP Error: " + String(httpCode));
    }
    http.end();
  } else {
    lv_label_set_text(reminder_label, "WiFi disconnected!");
  }
  
  show_loading(false); // Hide spinner, show label
}

/********************* Button Setup *********************/
void create_button() {
  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn, 120, 50);
  lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);
  
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "Get Reminder");
  lv_obj_center(label);
  
  lv_obj_add_event_cb(btn, [](lv_event_t *e) {
    fetch_reminder_data();
  }, LV_EVENT_CLICKED, NULL);
}

/********************* Main Setup *********************/
void setup() {
  Serial.begin(115200);
  connect_wifi();
  setup_lvgl();
  
  // Create loading spinner
  spinner = lv_spinner_create(lv_scr_act(), 1000, 60);
  lv_obj_set_size(spinner, 50, 50);
  lv_obj_center(spinner);
  
  // Create reminder label (initially hidden)
  reminder_label = lv_label_create(lv_scr_act());
  lv_label_set_text(reminder_label, "Reminder will appear here");
  lv_obj_set_width(reminder_label, TFT_WIDTH - 20);
  lv_obj_set_style_text_align(reminder_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(reminder_label);
  lv_obj_add_flag(reminder_label, LV_OBJ_FLAG_HIDDEN);
  
  create_button(); // Add the trigger button
}

void loop() {
  lv_timer_handler(); // Handle LVGL tasks
  delay(LVGL_TICK_PERIOD);
}