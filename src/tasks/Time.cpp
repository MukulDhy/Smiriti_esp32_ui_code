#include "time.h"
#include <Arduino.h>

unsigned long TimeManager::lastUpdateTime = 0;
struct tm TimeManager::timeInfo;
const int TimeManager::IST_OFFSET = 19800; // 5.5 hours in seconds
bool TimeManager::initialized = false;
bool TimeManager::timeValid = false;

void TimeManager::initialize()
{
    if (initialized)
        return; // Prevent multiple initialization

    // Set timezone to IST
    setenv("TZ", "IST-5:30", 1);
    tzset();

    // Initialize with a default time to prevent invalid time issues
    time_t rawtime;
    time(&rawtime);

    // If time is not set (before 2020), set a default time
    if (rawtime < 1577836800)
    { // Jan 1, 2020
        // Set to April 1, 2025, 19:04:00 IST as fallback
        setIndianTime(2025, 4, 1, 19, 4, 0);
    }
    else
    {
        localtime_r(&rawtime, &timeInfo);
        timeValid = true;
    }

    initialized = true;

    Serial.println("TimeManager initialized successfully");

    // Don't update display here - wait for proper UI initialization
}

void TimeManager::setIndianTime(int year, int month, int day, int hour, int min, int sec)
{
    if (!initialized)
    {
        Serial.println("TimeManager not initialized");
        return;
    }

    // Validate input parameters
    if (year < 2020 || year > 2050 || month < 1 || month > 12 ||
        day < 1 || day > 31 || hour < 0 || hour > 23 ||
        min < 0 || min > 59 || sec < 0 || sec > 59)
    {
        Serial.println("Invalid time parameters");
        return;
    }

    struct tm tm = {0}; // Initialize all fields to 0
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_isdst = -1; // Let system determine DST

    // Convert to time_t
    time_t t = mktime(&tm);
    if (t == -1)
    {
        Serial.println("Failed to convert time");
        return;
    }

    // Set system time
    struct timeval tv = {t, 0};
    if (settimeofday(&tv, nullptr) == 0)
    {
        // Update our cached time info
        localtime_r(&t, &timeInfo);
        timeValid = true;

        Serial.printf("Time set to: %04d-%02d-%02d %02d:%02d:%02d IST\n",
                      year, month, day, hour, min, sec);
    }
    else
    {
        Serial.println("Failed to set system time");
    }
}

String TimeManager::getFormattedTime()
{
    if (!timeValid)
        return "Invalid Time";

    char timeStr[32];
    snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02dT%02d:%02d:%02dZ",
             1900 + timeInfo.tm_year,
             timeInfo.tm_mon + 1,
             timeInfo.tm_mday,
             timeInfo.tm_hour,
             timeInfo.tm_min,
             timeInfo.tm_sec);
    return String(timeStr);
}

void TimeManager::update()
{
    if (!initialized)
        return;

    unsigned long currentMillis = millis();

    // Only update once per second
    if (currentMillis - lastUpdateTime < 1000)
    {
        return;
    }
    lastUpdateTime = currentMillis;

    // Get current time
    time_t rawtime;
    time(&rawtime);

    // Validate time
    if (rawtime < 1577836800)
    { // Before 2020
        Serial.println("Invalid system time detected");
        timeValid = false;
        return;
    }

    // Convert to local time
    if (localtime_r(&rawtime, &timeInfo) == nullptr)
    {
        Serial.println("Failed to convert time");
        timeValid = false;
        return;
    }

    timeValid = true;

    // CRITICAL FIX: Thread-safe UI updates with proper validation
    updateTimeDisplaySafe();

    // Update date only when it changes (optimization)
    static int lastDay = -1;
    if (timeInfo.tm_mday != lastDay)
    {
        updateDateDisplaySafe();
        lastDay = timeInfo.tm_mday;
    }
}

void TimeManager::updateTimeDisplaySafe()
{
    extern SemaphoreHandle_t lvglMutex;

    if (!timeValid || !lvglMutex)
    {
        return;
    }

    // Try to take mutex with shorter timeout
    if (xSemaphoreTake(lvglMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        // CRITICAL: Check rendering state first
        lv_display_t *disp = lv_display_get_default();
        if (disp && disp->rendering_in_progress)
        {
            // Don't wait, just skip this update
            xSemaphoreGive(lvglMutex);
            return;
        }

        lv_obj_t *current_screen = lv_scr_act();

        // Only update if on homepage and elements exist
        if (current_screen == ui_screen_homepage &&
            ui_homepage_label_time &&
            lv_obj_is_valid(ui_homepage_label_time))
        {
            // Additional check - make sure object is on current screen
            lv_obj_t *parent = lv_obj_get_screen(ui_homepage_label_time);
            if (parent == current_screen)
            {
                char timeStr[16];
                int result = snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d",
                                      timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

                if (result > 0 && result < sizeof(timeStr))
                {
                    lv_label_set_text(ui_homepage_label_time, timeStr);
                }
            }
        }

        xSemaphoreGive(lvglMutex);
    }
    // Don't log failure here to avoid spam
}

void TimeManager::updateDateDisplaySafe()
{
    extern SemaphoreHandle_t lvglMutex;

    if (!timeValid || !lvglMutex)
    {
        return;
    }

    static const char *monthNames[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    // Validate month index
    if (timeInfo.tm_mon < 0 || timeInfo.tm_mon > 11)
    {
        return;
    }

    // Try to take mutex with shorter timeout
    if (xSemaphoreTake(lvglMutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        // CRITICAL: Check rendering state first
        lv_display_t *disp = lv_display_get_default();
        if (disp && disp->rendering_in_progress)
        {
            // Don't wait, just skip this update
            xSemaphoreGive(lvglMutex);
            return;
        }

        lv_obj_t *current_screen = lv_scr_act();

        // Only update if on homepage and elements exist
        if (current_screen == ui_screen_homepage &&
            ui_homepage_label_date &&
            lv_obj_is_valid(ui_homepage_label_date))
        {
            // Additional check - make sure object is on current screen
            lv_obj_t *parent = lv_obj_get_screen(ui_homepage_label_date);
            if (parent == current_screen)
            {
                char dateStr[32];
                int result = snprintf(dateStr, sizeof(dateStr), "%d %s %d",
                                      timeInfo.tm_mday,
                                      monthNames[timeInfo.tm_mon],
                                      1900 + timeInfo.tm_year);

                if (result > 0 && result < sizeof(dateStr))
                {
                    lv_label_set_text(ui_homepage_label_date, dateStr);
                }
            }
        }

        xSemaphoreGive(lvglMutex);
    }
    // Don't log failure here to avoid spam
}
// Legacy functions for compatibility - now call safe versions
void TimeManager::updateTimeDisplay()
{
    updateTimeDisplaySafe();
}

void TimeManager::updateDateDisplay()
{
    updateDateDisplaySafe();
}

// Additional utility functions
bool TimeManager::isTimeValid()
{
    return timeValid && initialized;
}

void TimeManager::forceUpdate()
{
    lastUpdateTime = 0; // Force next update
}

// Emergency recovery function
void TimeManager::reset()
{
    initialized = false;
    timeValid = false;
    lastUpdateTime = 0;
    memset(&timeInfo, 0, sizeof(timeInfo));
}