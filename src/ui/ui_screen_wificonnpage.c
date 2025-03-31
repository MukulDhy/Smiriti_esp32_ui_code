// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.1
// LVGL version: 9.1.0
// Project name: Smriti_UI

#include "ui.h"

void ui_screen_wificonnpage_screen_init(void)
{
    ui_screen_wificonnpage = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_screen_wificonnpage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_wificonnpage_panel_backpannel5 = lv_obj_create(ui_screen_wificonnpage);
    lv_obj_set_width(ui_wificonnpage_panel_backpannel5, 188);
    lv_obj_set_height(ui_wificonnpage_panel_backpannel5, 50);
    lv_obj_set_x(ui_wificonnpage_panel_backpannel5, 2);
    lv_obj_set_y(ui_wificonnpage_panel_backpannel5, -101);
    lv_obj_set_align(ui_wificonnpage_panel_backpannel5, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_wificonnpage_panel_backpannel5, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_wificonnpage_label_back5 = lv_label_create(ui_wificonnpage_panel_backpannel5);
    lv_obj_set_width(ui_wificonnpage_label_back5, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_wificonnpage_label_back5, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_wificonnpage_label_back5, 10);
    lv_obj_set_y(ui_wificonnpage_label_back5, 5);
    lv_obj_set_align(ui_wificonnpage_label_back5, LV_ALIGN_CENTER);
    lv_label_set_text(ui_wificonnpage_label_back5, "Back");

    ui_wificonnpage_image_image6 = lv_image_create(ui_wificonnpage_panel_backpannel5);
    lv_image_set_src(ui_wificonnpage_image_image6, &ui_img_img_back_png);
    lv_obj_set_width(ui_wificonnpage_image_image6, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_wificonnpage_image_image6, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_wificonnpage_image_image6, -27);
    lv_obj_set_y(ui_wificonnpage_image_image6, 5);
    lv_obj_set_align(ui_wificonnpage_image_image6, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_wificonnpage_image_image6, LV_OBJ_FLAG_CLICKABLE);     /// Flags
    lv_obj_remove_flag(ui_wificonnpage_image_image6, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_wificonnpage_label_label10 = lv_label_create(ui_screen_wificonnpage);
    lv_obj_set_width(ui_wificonnpage_label_label10, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_wificonnpage_label_label10, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_wificonnpage_label_label10, 4);
    lv_obj_set_y(ui_wificonnpage_label_label10, -55);
    lv_obj_set_align(ui_wificonnpage_label_label10, LV_ALIGN_CENTER);
    lv_label_set_text(ui_wificonnpage_label_label10, "Wifi Conn page");
    lv_obj_set_style_text_color(ui_wificonnpage_label_label10, lv_color_hex(0x00FF17), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_wificonnpage_label_label10, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_wificonnpage_container_container2 = lv_obj_create(ui_screen_wificonnpage);
    lv_obj_remove_style_all(ui_wificonnpage_container_container2);
    lv_obj_set_width(ui_wificonnpage_container_container2, 196);
    lv_obj_set_height(ui_wificonnpage_container_container2, 29);
    lv_obj_set_x(ui_wificonnpage_container_container2, -1);
    lv_obj_set_y(ui_wificonnpage_container_container2, -21);
    lv_obj_set_align(ui_wificonnpage_container_container2, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_wificonnpage_container_container2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_wificonnpage_label_label13 = lv_label_create(ui_wificonnpage_container_container2);
    lv_obj_set_width(ui_wificonnpage_label_label13, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_wificonnpage_label_label13, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_wificonnpage_label_label13, -49);
    lv_obj_set_y(ui_wificonnpage_label_label13, -4);
    lv_obj_set_align(ui_wificonnpage_label_label13, LV_ALIGN_CENTER);
    lv_label_set_text(ui_wificonnpage_label_label13, "Wifi Status : ");

    ui_wificonnpage_label_label14 = lv_label_create(ui_wificonnpage_container_container2);
    lv_obj_set_width(ui_wificonnpage_label_label14, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_wificonnpage_label_label14, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_wificonnpage_label_label14, 42);
    lv_obj_set_y(ui_wificonnpage_label_label14, -4);
    lv_obj_set_align(ui_wificonnpage_label_label14, LV_ALIGN_CENTER);
    lv_label_set_text(ui_wificonnpage_label_label14, "Connected");
    lv_obj_set_style_text_color(ui_wificonnpage_label_label14, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_wificonnpage_label_label14, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_wificonnpage_label_label14, LV_TEXT_ALIGN_AUTO, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_wificonnpage_container_container3 = lv_obj_create(ui_screen_wificonnpage);
    lv_obj_remove_style_all(ui_wificonnpage_container_container3);
    lv_obj_set_width(ui_wificonnpage_container_container3, 196);
    lv_obj_set_height(ui_wificonnpage_container_container3, 29);
    lv_obj_set_x(ui_wificonnpage_container_container3, 1);
    lv_obj_set_y(ui_wificonnpage_container_container3, 8);
    lv_obj_set_align(ui_wificonnpage_container_container3, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_wificonnpage_container_container3, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_wificonnpage_label_label20 = lv_label_create(ui_wificonnpage_container_container3);
    lv_obj_set_width(ui_wificonnpage_label_label20, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_wificonnpage_label_label20, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_wificonnpage_label_label20, -49);
    lv_obj_set_y(ui_wificonnpage_label_label20, -4);
    lv_obj_set_align(ui_wificonnpage_label_label20, LV_ALIGN_CENTER);
    lv_label_set_text(ui_wificonnpage_label_label20, "Http Status : ");

    ui_wificonnpage_label_label21 = lv_label_create(ui_wificonnpage_container_container3);
    lv_obj_set_width(ui_wificonnpage_label_label21, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_wificonnpage_label_label21, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_wificonnpage_label_label21, 42);
    lv_obj_set_y(ui_wificonnpage_label_label21, -4);
    lv_obj_set_align(ui_wificonnpage_label_label21, LV_ALIGN_CENTER);
    lv_label_set_text(ui_wificonnpage_label_label21, "200");
    lv_obj_set_style_text_color(ui_wificonnpage_label_label21, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_wificonnpage_label_label21, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_wificonnpage_label_label21, LV_TEXT_ALIGN_AUTO, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_wificonnpage_panel_backpannel5, ui_event_wificonnpage_panel_backpannel5, LV_EVENT_ALL, NULL);

}
