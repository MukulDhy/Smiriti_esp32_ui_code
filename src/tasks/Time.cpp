#include "time.h"
#include <WiFi.h>

RTC_DS3231 TimeManager::rtc;
DateTime TimeManager::lastUpdate;
bool TimeManager::rtcFound = false;

// NTP Configuration
const char *TimeManager::NTP_SERVERS[] = {
    "pool.ntp.org",
    "time.google.com",
    "time.nist.gov",
    "time.windows.com",
    "1.in.pool.ntp.org",
    NULL // Sentinel
};

const long TimeManager::GMT_OFFSET = 19800; // GMT+5:30 (19800 seconds)
const int TimeManager::DAYLIGHT_OFFSET = 0;

bool TimeManager::begin()
{
    rtcFound = rtc.begin();

    if (!rtcFound)
    {
        Serial.println("Couldn't find RTC");
        lv_label_set_text(ui_homepage_label_time, "RTC Error");
        lv_label_set_text(ui_homepage_label_date, "RTC Error");
        return false;
    }

    if (rtc.lostPower())
    {
        Serial.println("RTC lost power, setting default time");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    lastUpdate = rtc.now();
    updateTimeDisplay(lastUpdate);
    updateDateDisplay(lastUpdate);

    return true;
}

bool TimeManager::tryNTPSync(const char *server)
{
    configTime(GMT_OFFSET, DAYLIGHT_OFFSET, server);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 5000))
    { // 5 second timeout
        Serial.printf("Failed to sync with %s\n", server);
        return false;
    }

    DateTime ntpTime(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec);

    rtc.adjust(ntpTime);
    lastUpdate = ntpTime;

    Serial.printf("Time synchronized with %s\n", server);
    return true;
}

bool TimeManager::syncWithNTP()
{
    if (!WiFi.isConnected())
        return false;

    // Try each server until one succeeds
    for (int i = 0; NTP_SERVERS[i] != NULL; i++)
    {
        if (tryNTPSync(NTP_SERVERS[i]))
        {
            return true;
        }
        delay(1000); // Brief delay between attempts
    }

    Serial.println("All NTP servers failed");
    return false;
}

void TimeManager::update()
{
    if (!rtcFound || lv_scr_act() != ui_screen_homepage)
        return;

    DateTime now = rtc.now();

    if (now.second() != lastUpdate.second())
    {
        updateTimeDisplay(now);
    }

    if (now.day() != lastUpdate.day() ||
        now.month() != lastUpdate.month() ||
        now.year() != lastUpdate.year())
    {
        updateDateDisplay(now);
    }

    lastUpdate = now;
}

const char *TimeManager::monthName(uint8_t month)
{
    static const char *months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    return (month >= 1 && month <= 12) ? months[month - 1] : "???";
}

void TimeManager::updateTimeDisplay(const DateTime &now)
{
    static char timeStr[9]; // HH:MM:SS + null
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d",
             now.hour(), now.minute(), now.second());
    lv_label_set_text(ui_homepage_label_time, timeStr);
}

void TimeManager::updateDateDisplay(const DateTime &now)
{
    static char dateStr[12]; // DD MMM YYYY + null
    snprintf(dateStr, sizeof(dateStr), "%02d %s %04d",
             now.day(), monthName(now.month()), now.year());
    lv_label_set_text(ui_homepage_label_date, dateStr);
}