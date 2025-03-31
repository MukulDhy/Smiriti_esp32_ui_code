#ifndef WIFI_H
#define WIFI_H

#include <WiFi.h>
#include <lvgl.h>
#include "ui/ui.h"

// Configuration
#define WIFI_DEBOUNCE_TIME_MS 3000   // 3-second debounce for disconnections
#define WIFI_RECONNECT_INTERVAL 5000 // 5 seconds between reconnect attempts
#define MAX_RECONNECT_ATTEMPTS 3     // Max retries before showing failure

class WiFiManager
{
public:
    enum class State
    {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        FAILED
    };

    static void begin(const char *ssid, const char *password);
    static void handle();
    static State getState();
    static String getStatus();

private:
    static void eventHandler(WiFiEvent_t event);
    static void updateUI();

    static State state;
    static String status;
    static const char *currentSSID;
    static const char *currentPassword;
    static unsigned long lastDisconnectTime;
    static unsigned long lastReconnectAttempt;
    static uint8_t reconnectAttempts;
};

#endif