// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.1
// LVGL version: 9.1.0
// Project name: Smriti_UI

#include "ui.h"

void ui_screen_wificonnectionpage1_screen_init(void)
{
    ui_screen_wificonnectionpage1 = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_screen_wificonnectionpage1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_wificonnectionpage1_label_title = lv_label_create(ui_screen_wificonnectionpage1);
    lv_obj_set_width(ui_wificonnectionpage1_label_title, 174);
    lv_obj_set_height(ui_wificonnectionpage1_label_title, 32);
    lv_obj_set_y(ui_wificonnectionpage1_label_title, -5);
    lv_obj_set_x(ui_wificonnectionpage1_label_title, lv_pct(1));
    lv_obj_set_align(ui_wificonnectionpage1_label_title, LV_ALIGN_CENTER);
    lv_label_set_text(ui_wificonnectionpage1_label_title, "Connecting....");
    lv_obj_set_style_text_font(ui_wificonnectionpage1_label_title, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_wificonnectionpage1_label_status = lv_label_create(ui_screen_wificonnectionpage1);
    lv_obj_set_width(ui_wificonnectionpage1_label_status, 172);
    lv_obj_set_height(ui_wificonnectionpage1_label_status, 57);
    lv_obj_set_y(ui_wificonnectionpage1_label_status, 50);
    lv_obj_set_x(ui_wificonnectionpage1_label_status, lv_pct(0));
    lv_obj_set_align(ui_wificonnectionpage1_label_status, LV_ALIGN_CENTER);
    lv_label_set_text(ui_wificonnectionpage1_label_status, "Disconnected");
    lv_obj_set_style_text_color(ui_wificonnectionpage1_label_status, lv_color_hex(0xFF0000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_wificonnectionpage1_label_status, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_wificonnectionpage1_label_status, &lv_font_montserrat_24,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_wificonnectionpage1_label_label6 = lv_label_create(ui_screen_wificonnectionpage1);
    lv_obj_set_width(ui_wificonnectionpage1_label_label6, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_wificonnectionpage1_label_label6, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_wificonnectionpage1_label_label6, -5);
    lv_obj_set_y(ui_wificonnectionpage1_label_label6, -43);
    lv_obj_set_align(ui_wificonnectionpage1_label_label6, LV_ALIGN_CENTER);
    lv_label_set_text(ui_wificonnectionpage1_label_label6, "Wifi");
    lv_obj_set_style_text_color(ui_wificonnectionpage1_label_label6, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_wificonnectionpage1_label_label6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_wificonnectionpage1_label_label6, &lv_font_montserrat_20,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_wificonnectionpage1_label_label11 = lv_label_create(ui_screen_wificonnectionpage1);
    lv_obj_set_width(ui_wificonnectionpage1_label_label11, 130);
    lv_obj_set_height(ui_wificonnectionpage1_label_label11, 20);
    lv_obj_set_x(ui_wificonnectionpage1_label_label11, -7);
    lv_obj_set_y(ui_wificonnectionpage1_label_label11, -72);
    lv_obj_set_align(ui_wificonnectionpage1_label_label11, LV_ALIGN_CENTER);
    lv_label_set_text(ui_wificonnectionpage1_label_label11, "Starting App");
    lv_obj_set_style_text_align(ui_wificonnectionpage1_label_label11, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_wificonnectionpage1_label_title, ui_event_wificonnectionpage1_label_title, LV_EVENT_ALL, NULL);
    uic_Screen_wificonnectionpage1 = ui_screen_wificonnectionpage1;
    uic_wificonnectionpage1_Label_status = ui_wificonnectionpage1_label_status;

}
