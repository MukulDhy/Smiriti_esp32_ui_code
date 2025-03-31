#include "wifi.h"

WiFiManager::State WiFiManager::state = WiFiManager::State::DISCONNECTED;
String WiFiManager::status = "Disconnected";
const char *WiFiManager::currentSSID = nullptr;
const char *WiFiManager::currentPassword = nullptr;
unsigned long WiFiManager::lastDisconnectTime = 0;
unsigned long WiFiManager::lastReconnectAttempt = 0;
uint8_t WiFiManager::reconnectAttempts = 0;

void WiFiManager::begin(const char *ssid, const char *password)
{
    currentSSID = ssid;
    currentPassword = password;

    WiFi.onEvent(WiFiManager::eventHandler);
    WiFi.setAutoReconnect(false); // We handle reconnection manually
    WiFi.begin(ssid, password);

    state = State::CONNECTING;
    status = "Connecting...";
    updateUI();
}

void WiFiManager::eventHandler(WiFiEvent_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        lastDisconnectTime = millis();
        state = State::DISCONNECTED;
        status = "Disconnected";
        reconnectAttempts++;
        updateUI();
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        state = State::CONNECTED;
        status = "Connected: " + WiFi.localIP().toString();
        reconnectAttempts = 0;
        updateUI();
        if (lv_scr_act() != ui_screen_homepage)
        {
            _ui_screen_change(&ui_screen_homepage, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, NULL);
        }
        break;

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        state = State::CONNECTING;
        status = "Authenticating...";
        updateUI();
        break;
    }
}

void WiFiManager::handle()
{
    // Handle debounced disconnection
    if (state == State::CONNECTED && WiFi.status() != WL_CONNECTED)
    {
        if (millis() - lastDisconnectTime > WIFI_DEBOUNCE_TIME_MS)
        {
            state = State::DISCONNECTED;
            status = "Disconnected";
            updateUI();
        }
        return;
    }

    // Handle reconnection logic
    if (state == State::DISCONNECTED)
    {
        if (reconnectAttempts >= MAX_RECONNECT_ATTEMPTS)
        {
            state = State::FAILED;
            status = "Connection Failed";
            updateUI();
            return;
        }

        if (millis() - lastReconnectAttempt > WIFI_RECONNECT_INTERVAL)
        {
            WiFi.begin(currentSSID, currentPassword);
            state = State::CONNECTING;
            status = "Reconnecting...";
            lastReconnectAttempt = millis();
            updateUI();
        }
    }

    // Update connecting animation
    if (state == State::CONNECTING)
    {
        static uint8_t dots = 0;
        static unsigned long lastUpdate = 0;

        if (millis() - lastUpdate > 500)
        {
            dots = (dots + 1) % 4;
            String connectingText = "Connecting";
            for (uint8_t i = 0; i < dots; i++)
                connectingText += ".";
            status = connectingText;
            updateUI();
            lastUpdate = millis();
        }
    }
}

void WiFiManager::updateUI()
{
    if (lv_scr_act() != ui_screen_wificonnectionpage1 && state != State::CONNECTED)
    {
        _ui_screen_change(&ui_screen_wificonnectionpage1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, NULL);
    }

    lv_label_set_text(ui_wificonnectionpage1_label_status, status.c_str());
    lv_obj_set_style_text_color(
        ui_wificonnectionpage1_label_status,
        (state == State::CONNECTED) ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000),
        LV_PART_MAIN | LV_STATE_DEFAULT);
}

WiFiManager::State WiFiManager::getState() { return state; }
String WiFiManager::getStatus() { return status; }