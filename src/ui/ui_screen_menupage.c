// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.1
// LVGL version: 9.1.0
// Project name: Smriti_UI

#include "ui.h"

void ui_screen_menupage_screen_init(void)
{
    ui_screen_menupage = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_screen_menupage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_menupage_panel_backpannel = lv_obj_create(ui_screen_menupage);
    lv_obj_set_width(ui_menupage_panel_backpannel, 188);
    lv_obj_set_height(ui_menupage_panel_backpannel, 50);
    lv_obj_set_x(ui_menupage_panel_backpannel, 2);
    lv_obj_set_y(ui_menupage_panel_backpannel, -101);
    lv_obj_set_align(ui_menupage_panel_backpannel, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_menupage_panel_backpannel, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_menupage_label_back = lv_label_create(ui_menupage_panel_backpannel);
    lv_obj_set_width(ui_menupage_label_back, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_menupage_label_back, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_menupage_label_back, 10);
    lv_obj_set_y(ui_menupage_label_back, 5);
    lv_obj_set_align(ui_menupage_label_back, LV_ALIGN_CENTER);
    lv_label_set_text(ui_menupage_label_back, "Back");

    ui_menupage_image_image1 = lv_image_create(ui_menupage_panel_backpannel);
    lv_image_set_src(ui_menupage_image_image1, &ui_img_img_back_png);
    lv_obj_set_width(ui_menupage_image_image1, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_menupage_image_image1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_menupage_image_image1, -27);
    lv_obj_set_y(ui_menupage_image_image1, 5);
    lv_obj_set_align(ui_menupage_image_image1, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_menupage_image_image1, LV_OBJ_FLAG_CLICKABLE);     /// Flags
    lv_obj_remove_flag(ui_menupage_image_image1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_menupage_label_label5 = lv_label_create(ui_screen_menupage);
    lv_obj_set_width(ui_menupage_label_label5, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_menupage_label_label5, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_menupage_label_label5, 0);
    lv_obj_set_y(ui_menupage_label_label5, -61);
    lv_obj_set_align(ui_menupage_label_label5, LV_ALIGN_CENTER);
    lv_label_set_text(ui_menupage_label_label5, "Menu Page");
    lv_obj_set_style_text_color(ui_menupage_label_label5, lv_color_hex(0x00FF17), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_menupage_label_label5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_menupage_panel_panel3 = lv_obj_create(ui_screen_menupage);
    lv_obj_set_width(ui_menupage_panel_panel3, 202);
    lv_obj_set_height(ui_menupage_panel_panel3, 105);
    lv_obj_set_x(ui_menupage_panel_panel3, 4);
    lv_obj_set_y(ui_menupage_panel_panel3, 9);
    lv_obj_set_align(ui_menupage_panel_panel3, LV_ALIGN_CENTER);
    lv_obj_set_scroll_dir(ui_menupage_panel_panel3, LV_DIR_VER);

    ui_menupage_button_button1 = lv_button_create(ui_menupage_panel_panel3);
    lv_obj_set_width(ui_menupage_button_button1, 78);
    lv_obj_set_height(ui_menupage_button_button1, 36);
    lv_obj_set_x(ui_menupage_button_button1, -48);
    lv_obj_set_y(ui_menupage_button_button1, -18);
    lv_obj_set_align(ui_menupage_button_button1, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_menupage_button_button1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_remove_flag(ui_menupage_button_button1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_menupage_label_label2 = lv_label_create(ui_menupage_button_button1);
    lv_obj_set_width(ui_menupage_label_label2, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_menupage_label_label2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_menupage_label_label2, LV_ALIGN_CENTER);
    lv_label_set_text(ui_menupage_label_label2, "reminder");

    ui_menupage_button_button2 = lv_button_create(ui_menupage_panel_panel3);
    lv_obj_set_width(ui_menupage_button_button2, 78);
    lv_obj_set_height(ui_menupage_button_button2, 36);
    lv_obj_set_x(ui_menupage_button_button2, 45);
    lv_obj_set_y(ui_menupage_button_button2, -21);
    lv_obj_set_align(ui_menupage_button_button2, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_menupage_button_button2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_remove_flag(ui_menupage_button_button2, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_menupage_label_label1 = lv_label_create(ui_menupage_button_button2);
    lv_obj_set_width(ui_menupage_label_label1, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_menupage_label_label1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_menupage_label_label1, LV_ALIGN_CENTER);
    lv_label_set_text(ui_menupage_label_label1, "Alerts");

    ui_menupage_button_button3 = lv_button_create(ui_menupage_panel_panel3);
    lv_obj_set_width(ui_menupage_button_button3, 78);
    lv_obj_set_height(ui_menupage_button_button3, 36);
    lv_obj_set_x(ui_menupage_button_button3, -47);
    lv_obj_set_y(ui_menupage_button_button3, 26);
    lv_obj_set_align(ui_menupage_button_button3, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_menupage_button_button3, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_remove_flag(ui_menupage_button_button3, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_menupage_label_label3 = lv_label_create(ui_menupage_button_button3);
    lv_obj_set_width(ui_menupage_label_label3, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_menupage_label_label3, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_menupage_label_label3, LV_ALIGN_CENTER);
    lv_label_set_text(ui_menupage_label_label3, "Profile");

    ui_menupage_button_button4 = lv_button_create(ui_menupage_panel_panel3);
    lv_obj_set_width(ui_menupage_button_button4, 78);
    lv_obj_set_height(ui_menupage_button_button4, 36);
    lv_obj_set_x(ui_menupage_button_button4, 45);
    lv_obj_set_y(ui_menupage_button_button4, 25);
    lv_obj_set_align(ui_menupage_button_button4, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_menupage_button_button4, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_remove_flag(ui_menupage_button_button4, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_menupage_label_label4 = lv_label_create(ui_menupage_button_button4);
    lv_obj_set_width(ui_menupage_label_label4, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_menupage_label_label4, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_menupage_label_label4, LV_ALIGN_CENTER);
    lv_label_set_text(ui_menupage_label_label4, "Wifi Conn");

    ui_menupage_button_button8 = lv_button_create(ui_menupage_panel_panel3);
    lv_obj_set_width(ui_menupage_button_button8, 78);
    lv_obj_set_height(ui_menupage_button_button8, 36);
    lv_obj_set_x(ui_menupage_button_button8, -47);
    lv_obj_set_y(ui_menupage_button_button8, 73);
    lv_obj_set_align(ui_menupage_button_button8, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_menupage_button_button8, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_remove_flag(ui_menupage_button_button8, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_menupage_label_label32 = lv_label_create(ui_menupage_button_button8);
    lv_obj_set_width(ui_menupage_label_label32, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_menupage_label_label32, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_menupage_label_label32, LV_ALIGN_CENTER);
    lv_label_set_text(ui_menupage_label_label32, "Remind");

    ui_menupage_button_button10 = lv_button_create(ui_menupage_panel_panel3);
    lv_obj_set_width(ui_menupage_button_button10, 78);
    lv_obj_set_height(ui_menupage_button_button10, 36);
    lv_obj_set_x(ui_menupage_button_button10, 44);
    lv_obj_set_y(ui_menupage_button_button10, 73);
    lv_obj_set_align(ui_menupage_button_button10, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_menupage_button_button10, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_remove_flag(ui_menupage_button_button10, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_menupage_label_label34 = lv_label_create(ui_menupage_button_button10);
    lv_obj_set_width(ui_menupage_label_label34, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_menupage_label_label34, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_menupage_label_label34, LV_ALIGN_CENTER);
    lv_label_set_text(ui_menupage_label_label34, "Profile");

    lv_obj_add_event_cb(ui_menupage_panel_backpannel, ui_event_menupage_panel_backpannel, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_menupage_button_button1, ui_event_menupage_button_button1, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_menupage_button_button2, ui_event_menupage_button_button2, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_menupage_button_button3, ui_event_menupage_button_button3, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_menupage_button_button4, ui_event_menupage_button_button4, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_menupage_button_button8, ui_event_menupage_button_button8, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_menupage_button_button10, ui_event_menupage_button_button10, LV_EVENT_ALL, NULL);

}
