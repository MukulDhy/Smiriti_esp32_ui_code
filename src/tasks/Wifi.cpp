// wifi.cpp
#include "wifi.h"
#include <WiFi.h>
#include <ui/ui.h>

const char *ssid = "Mukuldhy";
const char *password = "12345678";

enum WiFiState
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    RECONNECTING
};
WiFiState wifiState = DISCONNECTED;
unsigned long wifiTimeout = 0;
uint8_t reconnectAttempts = 0;
String connectionStatus;

void updateStatusLabel()
{
    lv_label_set_text(ui_wificonnectionpage1_label_status, connectionStatus.c_str());
    lv_obj_set_style_text_color(ui_wificonnectionpage1_label_status,
                                (wifiState == CONNECTED) ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
}

void initWiFi()
{
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.begin(ssid, password);
    wifiState = CONNECTING;
    wifiTimeout = millis() + 10000;
}

void handleWiFi()
{
    static unsigned long lastUpdate = 0;
    static uint8_t dots = 0;

    if (WiFi.status() == WL_CONNECTED)
    {
        if (wifiState != CONNECTED)
        {
            connectionStatus = "Connected!";
            wifiState = CONNECTED;
            lv_scr_load(ui_screen_homepage);
            updateStatusLabel();
        }
        return;
    }

    // State handling
    switch (wifiState)
    {
    case CONNECTING:
        if (millis() - lastUpdate > 500)
        {
            // Create dot animation without String.repeat()
            connectionStatus = "Connecting";
            dots = (dots + 1) % 4;
            for (uint8_t i = 0; i < dots; i++)
            {
                connectionStatus += ".";
            }

            updateStatusLabel();
            lastUpdate = millis();
        }
        if (millis() > wifiTimeout)
        {
            wifiState = RECONNECTING;
            reconnectAttempts = 0;
        }
        break;

    case RECONNECTING:
        if (reconnectAttempts >= 3)
        {
            connectionStatus = "Connection Failed";
            updateStatusLabel();
            wifiState = DISCONNECTED;
            return;
        }

        if (millis() - lastUpdate > 2000)
        {
            connectionStatus = "Reconnecting (";
            connectionStatus += String(reconnectAttempts + 1);
            connectionStatus += "/3)";
            updateStatusLabel();
            WiFi.reconnect();
            lastUpdate = millis();
            wifiTimeout = millis() + 10000;
        }
        break;

    case DISCONNECTED:
        lv_disp_load_scr(ui_screen_wificonnpage);
        if (millis() - lastUpdate > 5000)
        {
            wifiState = CONNECTING;
            initWiFi();
            lastUpdate = millis();
        }
        break;
    }
}