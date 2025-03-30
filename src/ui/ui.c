// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.1
// LVGL version: 9.1.0
// Project name: Smriti_UI

#include "ui.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////

// SCREEN: ui_screen_wificonnectionpage1
void ui_screen_wificonnectionpage1_screen_init(void);
void ui_event_screen_wificonnectionpage1(lv_event_t * e);
lv_obj_t * ui_screen_wificonnectionpage1;
lv_obj_t * ui_wificonnectionpage1_label_title;
void ui_event_wificonnectionpage1_label_status(lv_event_t * e);
lv_obj_t * ui_wificonnectionpage1_label_status;
lv_obj_t * ui_wificonnectionpage1_label_statuserror;
// CUSTOM VARIABLES
lv_obj_t * uic_Screen_wificonnectionpage1;
lv_obj_t * uic_wificonnectionpage1_Label_status;
lv_obj_t * uic_wificonnectionpage1_Label_statuserror;

// SCREEN: ui_screen_homepage
void ui_screen_homepage_screen_init(void);
lv_obj_t * ui_screen_homepage;
lv_obj_t * ui_homepage_label_title;
lv_obj_t * ui_homepage_label_time;
void ui_event_homepage_button_menubutton(lv_event_t * e);
lv_obj_t * ui_homepage_button_menubutton;
void ui_event_homepage_label_menubtnlabel(lv_event_t * e);
lv_obj_t * ui_homepage_label_menubtnlabel;
// CUSTOM VARIABLES
lv_obj_t * uic_homepage_Label_menubtnLabel;

// SCREEN: ui_screen_menupage
void ui_screen_menupage_screen_init(void);
lv_obj_t * ui_screen_menupage;
void ui_event_menupage_panel_backpannel(lv_event_t * e);
lv_obj_t * ui_menupage_panel_backpannel;
lv_obj_t * ui_menupage_label_back;
lv_obj_t * ui_menupage_image_image1;
void ui_event_menupage_button_button1(lv_event_t * e);
lv_obj_t * ui_menupage_button_button1;
lv_obj_t * ui_menupage_label_label2;
void ui_event_menupage_button_button2(lv_event_t * e);
lv_obj_t * ui_menupage_button_button2;
lv_obj_t * ui_menupage_label_label1;
void ui_event_menupage_button_button3(lv_event_t * e);
lv_obj_t * ui_menupage_button_button3;
lv_obj_t * ui_menupage_label_label3;
void ui_event_menupage_button_button4(lv_event_t * e);
lv_obj_t * ui_menupage_button_button4;
lv_obj_t * ui_menupage_label_label4;
lv_obj_t * ui_menupage_label_label5;
// CUSTOM VARIABLES

// SCREEN: ui_screen_reminderpage
void ui_screen_reminderpage_screen_init(void);
lv_obj_t * ui_screen_reminderpage;
void ui_event_reminderpage_panel_backpannel2(lv_event_t * e);
lv_obj_t * ui_reminderpage_panel_backpannel2;
lv_obj_t * ui_reminderpage_label_back2;
lv_obj_t * ui_reminderpage_image_image3;
lv_obj_t * ui_reminderpage_label_label7;
// CUSTOM VARIABLES

// SCREEN: ui_screen_alertpage
void ui_screen_alertpage_screen_init(void);
lv_obj_t * ui_screen_alertpage;
void ui_event_alertpage_panel_backpannel3(lv_event_t * e);
lv_obj_t * ui_alertpage_panel_backpannel3;
lv_obj_t * ui_alertpage_label_back3;
lv_obj_t * ui_alertpage_image_image4;
lv_obj_t * ui_alertpage_label_label8;
// CUSTOM VARIABLES

// SCREEN: ui_screen_profilepage
void ui_screen_profilepage_screen_init(void);
lv_obj_t * ui_screen_profilepage;
void ui_event_profilepage_panel_backpannel4(lv_event_t * e);
lv_obj_t * ui_profilepage_panel_backpannel4;
lv_obj_t * ui_profilepage_label_back4;
lv_obj_t * ui_profilepage_image_image5;
lv_obj_t * ui_profilepage_label_label9;
// CUSTOM VARIABLES

// SCREEN: ui_screen_wificonnpage
void ui_screen_wificonnpage_screen_init(void);
lv_obj_t * ui_screen_wificonnpage;
void ui_event_wificonnpage_panel_backpannel5(lv_event_t * e);
lv_obj_t * ui_wificonnpage_panel_backpannel5;
lv_obj_t * ui_wificonnpage_label_back5;
lv_obj_t * ui_wificonnpage_image_image6;
lv_obj_t * ui_wificonnpage_label_label10;
// CUSTOM VARIABLES

// EVENTS
void ui_event_startevents____initial_actions0(lv_event_t * e);
lv_obj_t * ui_startevents____initial_actions0;

// IMAGES AND IMAGE SETS

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////
void ui_event_screen_wificonnectionpage1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_LEFT) {
        lv_indev_wait_release(lv_indev_active());
        _ui_screen_change(&ui_screen_homepage, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, &ui_screen_homepage_screen_init);
    }
}

void ui_event_wificonnectionpage1_label_status(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_screen_homepage, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, &ui_screen_homepage_screen_init);
    }
}

void ui_event_homepage_button_menubutton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_screen_menupage, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, &ui_screen_menupage_screen_init);
    }
}

void ui_event_homepage_label_menubtnlabel(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_screen_menupage, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_screen_menupage_screen_init);
    }
}

void ui_event_menupage_panel_backpannel(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_screen_homepage, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_screen_homepage_screen_init);
    }
}

void ui_event_menupage_button_button1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_screen_reminderpage, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, &ui_screen_reminderpage_screen_init);
    }
}

void ui_event_menupage_button_button2(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_screen_alertpage, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, &ui_screen_alertpage_screen_init);
    }
}

void ui_event_menupage_button_button3(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_screen_profilepage, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, &ui_screen_profilepage_screen_init);
    }
}

void ui_event_menupage_button_button4(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_screen_wificonnpage, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, &ui_screen_wificonnpage_screen_init);
    }
}

void ui_event_reminderpage_panel_backpannel2(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_screen_menupage, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_screen_menupage_screen_init);
    }
}

void ui_event_alertpage_panel_backpannel3(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_screen_menupage, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_screen_menupage_screen_init);
    }
}

void ui_event_profilepage_panel_backpannel4(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_screen_menupage, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_screen_menupage_screen_init);
    }
}

void ui_event_wificonnpage_panel_backpannel5(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_screen_menupage, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, &ui_screen_menupage_screen_init);
    }
}

void ui_event_startevents____initial_actions0(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_SCREEN_LOAD_START) {
        wifiConnectionStart(e);
    }
}

///////////////////// SCREENS ////////////////////

void ui_init(void)
{
    lv_disp_t * dispp = lv_display_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_screen_wificonnectionpage1_screen_init();
    ui_screen_homepage_screen_init();
    ui_screen_menupage_screen_init();
    ui_screen_reminderpage_screen_init();
    ui_screen_alertpage_screen_init();
    ui_screen_profilepage_screen_init();
    ui_screen_wificonnpage_screen_init();
    ui_startevents____initial_actions0 = lv_obj_create(NULL);
    lv_obj_add_event_cb(ui_startevents____initial_actions0, ui_event_startevents____initial_actions0, LV_EVENT_ALL, NULL);
    lv_disp_load_scr(ui_screen_wificonnectionpage1);
}
