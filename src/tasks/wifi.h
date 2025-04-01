#ifndef WIFI_H
#define WIFI_H

#include <WiFi.h>
#include <lvgl.h>
#include "ui/ui.h"

// Configuration with documentation
#define WIFI_DEBOUNCE_TIME_MS 3000   // Debounce time for disconnection events
#define WIFI_RECONNECT_INTERVAL 5000 // Interval between reconnect attempts
#define MAX_RECONNECT_ATTEMPTS 5     // Maximum retries before failure state
#define WIFI_CONNECT_TIMEOUT 30000   // 30s timeout for initial connection
#define WIFI_HANDLE_INTERVAL 100     // Main loop handling interval in ms

class WiFiManager
{
public:
    enum class State
    {
        DISCONNECTED, // Not connected to WiFi
        CONNECTING,   // Attempting to connect
        CONNECTED,    // Successfully connected with IP
        FAILED,       // Connection failed after retries
        RETRY,        // Manual retry requested
        TIMEOUT       // Connection attempt timed out
    };

    // Initialize WiFi connection
    static void begin(const char *ssid, const char *password);

    // Main handling function to be called in loop()
    static void handle();

    // Get current connection state
    static State getState();

    // Get current status message
    static String getStatus();

    // Manual retry function
    static void retryConnection();

    // Check if credentials are set
    static bool hasCredentials();

private:
    static void eventHandler(WiFiEvent_t event, WiFiEventInfo_t info);
    static void updateUI();
    static void resetConnection();
    static void handleConnectingState();
    static void handleDisconnectedState();
    static void handleConnectedState();

    // Connection state tracking
    static State state;
    static String status;
    static const char *currentSSID;
    static const char *currentPassword;
    static unsigned long lastDisconnectTime;
    static unsigned long lastReconnectAttempt;
    static unsigned long connectionStartTime;
    static uint8_t reconnectAttempts;
};

#endif