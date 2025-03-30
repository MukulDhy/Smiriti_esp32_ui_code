// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.1
// LVGL version: 9.1.0
// Project name: Smriti_UI

#include "ui.h"

void ui_screen_homepage_screen_init(void)
{
    ui_screen_homepage = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_screen_homepage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_homepage_label_title = lv_label_create(ui_screen_homepage);
    lv_obj_set_width(ui_homepage_label_title, 117);
    lv_obj_set_height(ui_homepage_label_title, 38);
    lv_obj_set_x(ui_homepage_label_title, -1);
    lv_obj_set_y(ui_homepage_label_title, -40);
    lv_obj_set_align(ui_homepage_label_title, LV_ALIGN_CENTER);
    lv_label_set_text(ui_homepage_label_title, "Smriti");
    lv_obj_set_style_text_color(ui_homepage_label_title, lv_color_hex(0x35E73C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_homepage_label_title, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_homepage_label_title, &lv_font_montserrat_36, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_homepage_label_time = lv_label_create(ui_screen_homepage);
    lv_obj_set_width(ui_homepage_label_time, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_homepage_label_time, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_homepage_label_time, -4);
    lv_obj_set_y(ui_homepage_label_time, 2);
    lv_obj_set_align(ui_homepage_label_time, LV_ALIGN_CENTER);
    lv_label_set_text(ui_homepage_label_time, "12:23:45");
    lv_obj_set_style_text_font(ui_homepage_label_time, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_homepage_button_menubutton = lv_button_create(ui_screen_homepage);
    lv_obj_set_width(ui_homepage_button_menubutton, 82);
    lv_obj_set_height(ui_homepage_button_menubutton, 30);
    lv_obj_set_x(ui_homepage_button_menubutton, -3);
    lv_obj_set_y(ui_homepage_button_menubutton, 47);
    lv_obj_set_align(ui_homepage_button_menubutton, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_homepage_button_menubutton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_remove_flag(ui_homepage_button_menubutton, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_homepage_button_menubutton, lv_color_hex(0x24D858), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_homepage_button_menubutton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_homepage_label_menubtnlabel = lv_label_create(ui_homepage_button_menubutton);
    lv_obj_set_width(ui_homepage_label_menubtnlabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_homepage_label_menubtnlabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_homepage_label_menubtnlabel, 0);
    lv_obj_set_y(ui_homepage_label_menubtnlabel, 1);
    lv_obj_set_align(ui_homepage_label_menubtnlabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_homepage_label_menubtnlabel, "Menu");
    lv_obj_set_style_text_font(ui_homepage_label_menubtnlabel, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_homepage_label_menubtnlabel, ui_event_homepage_label_menubtnlabel, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_homepage_button_menubutton, ui_event_homepage_button_menubutton, LV_EVENT_ALL, NULL);
    uic_homepage_Label_menubtnLabel = ui_homepage_label_menubtnlabel;

}
