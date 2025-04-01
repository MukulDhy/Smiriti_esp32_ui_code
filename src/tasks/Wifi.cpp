#include "wifi.h"

// Initialize static members
WiFiManager::State WiFiManager::state = WiFiManager::State::DISCONNECTED;
String WiFiManager::status = "Disconnected";
const char *WiFiManager::currentSSID = nullptr;
const char *WiFiManager::currentPassword = nullptr;
unsigned long WiFiManager::lastDisconnectTime = 0;
unsigned long WiFiManager::lastReconnectAttempt = 0;
unsigned long WiFiManager::connectionStartTime = 0;
uint8_t WiFiManager::reconnectAttempts = 0;

// UI Event Handler
void ui_event_wificonnectionpage1_button_button5(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if (event_code == LV_EVENT_CLICKED)
    {
        WiFiManager::retryConnection();
    }
}

void WiFiManager::begin(const char *ssid, const char *password)
{
    // Validate inputs
    if (ssid == nullptr || password == nullptr || strlen(ssid) == 0)
    {
        state = State::FAILED;
        status = "Invalid credentials";
        updateUI();
        return;
    }

    // Store credentials
    currentSSID = ssid;
    currentPassword = password;

    // Configure WiFi for robust operation
    WiFi.disconnect(true); // Clear previous connection
    WiFi.onEvent(WiFiManager::eventHandler);
    WiFi.setAutoReconnect(false);
    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    WiFi.setAutoConnect(false);

    // Start connection process
    WiFi.begin(ssid, password);
    connectionStartTime = millis();
    state = State::CONNECTING;
    status = "Connecting...";
    reconnectAttempts = 0;
    updateUI();
}

void WiFiManager::retryConnection()
{
    if (currentSSID && currentPassword)
    {
        state = State::RETRY;
        reconnectAttempts = 0;
        begin(currentSSID, currentPassword);
    }
}

void WiFiManager::resetConnection()
{
    WiFi.disconnect(true);
    delay(100); // Small delay to ensure disconnect completes
}

void WiFiManager::eventHandler(WiFiEvent_t event, WiFiEventInfo_t info)
{
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        lastDisconnectTime = millis();
        state = State::DISCONNECTED;
        status = "Disconnected";
        if (info.wifi_sta_disconnected.reason == WIFI_REASON_NO_AP_FOUND)
        {
            status = "Network not found";
        }
        else if (info.wifi_sta_disconnected.reason == WIFI_REASON_AUTH_FAIL)
        {
            status = "Authentication failed";
        }
        reconnectAttempts++;
        updateUI();
        break;

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        state = State::CONNECTING;
        status = "Authenticating...";
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
            delay(1000);
        }
        break;

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        state = State::DISCONNECTED;
        status = "Lost IP address";
        updateUI();
        break;

    default:
        // Ignore other events
        break;
    }
}

void WiFiManager::handle()
{
    static unsigned long lastHandleTime = 0;
    unsigned long currentTime = millis();

    // Throttle the handle frequency
    if (currentTime - lastHandleTime < WIFI_HANDLE_INTERVAL)
    {
        return;
    }
    lastHandleTime = currentTime;

    // State-specific handling
    switch (state)
    {
    case State::CONNECTING:
        handleConnectingState();
        break;
    case State::DISCONNECTED:
        handleDisconnectedState();
        break;
    case State::CONNECTED:
        handleConnectedState();
        break;
    case State::RETRY:
        retryConnection();
        break;
    default:
        // Other states don't need special handling
        break;
    }
}

void WiFiManager::handleConnectingState()
{
    unsigned long currentTime = millis();

    // Check for connection timeout
    if (currentTime - connectionStartTime > WIFI_CONNECT_TIMEOUT)
    {
        state = State::TIMEOUT;
        status = "Connection timed out";
        resetConnection();
        updateUI();
        return;
    }

    // Update connecting animation
    static uint8_t dots = 0;
    static unsigned long lastUpdate = 0;

    if (currentTime - lastUpdate > 500)
    {
        dots = (dots + 1) % 4;
        String baseText = status.startsWith("Reconnecting") ? "Reconnecting" : "Connecting";
        String connectingText = baseText;
        for (uint8_t i = 0; i < dots; i++)
        {
            connectingText += ".";
        }
        status = connectingText;
        updateUI();
        lastUpdate = currentTime;
    }
}

void WiFiManager::handleDisconnectedState()
{
    unsigned long currentTime = millis();

    if (reconnectAttempts >= MAX_RECONNECT_ATTEMPTS)
    {
        state = State::FAILED;
        status = "Connection Failed";
        if (ui_wificonnectionpage1_container_container4)
        {
            lv_obj_remove_flag(ui_wificonnectionpage1_container_container4, LV_OBJ_FLAG_HIDDEN);
        }
        updateUI();
        return;
    }

    if (currentTime - lastReconnectAttempt > WIFI_RECONNECT_INTERVAL)
    {
        resetConnection();
        WiFi.begin(currentSSID, currentPassword);
        state = State::CONNECTING;
        status = "Reconnecting...";
        lastReconnectAttempt = currentTime;
        connectionStartTime = currentTime;
        updateUI();
    }
}

void WiFiManager::handleConnectedState()
{
    // Verify connection is still active
    if (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - lastDisconnectTime > WIFI_DEBOUNCE_TIME_MS)
        {
            state = State::DISCONNECTED;
            status = "Disconnected";
            updateUI();
        }
    }
}

void WiFiManager::updateUI()
{
    // Check if UI elements exist before updating
    if (!ui_wificonnpage_label_label14 || !ui_wificonnectionpage1_label_status)
    {
        return;
    }

    // Only change screen if necessary
    if (lv_scr_act() != ui_screen_wificonnectionpage1 && state != State::CONNECTED)
    {
        _ui_screen_change(&ui_screen_wificonnectionpage1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0, NULL);
    }

    // Update status text
    lv_label_set_text(ui_wificonnpage_label_label14, status.c_str());
    lv_label_set_text(ui_wificonnectionpage1_label_status, status.c_str());

    // Update status color based on state
    lv_color_t statusColor;
    switch (state)
    {
    case State::CONNECTED:
        statusColor = lv_color_hex(0x00FF00); // Green
        break;
    case State::FAILED:
    case State::TIMEOUT:
        statusColor = lv_color_hex(0xFFA500); // Orange
        break;
    case State::CONNECTING:
        statusColor = lv_color_hex(0xFFFF00); // Yellow
        break;
    default:
        statusColor = lv_color_hex(0xFF0000); // Red
        break;
    }

    lv_obj_set_style_text_color(
        ui_wificonnectionpage1_label_status,
        statusColor,
        LV_PART_MAIN | LV_STATE_DEFAULT);
}

WiFiManager::State WiFiManager::getState()
{
    return state;
}

String WiFiManager::getStatus()
{
    return status;
}

bool WiFiManager::hasCredentials()
{
    return currentSSID != nullptr && strlen(currentSSID) > 0;
}
