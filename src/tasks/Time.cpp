#include "time.h"
#include <time.h>

unsigned long TimeManager::lastUpdateTime = 0;
struct tm TimeManager::timeInfo;
const int TimeManager::IST_OFFSET;

void TimeManager::initialize()
{
    // Set timezone to IST (doesn't work on all embedded systems)
    setenv("TZ", "IST-5:30", 1);
    tzset();

    // Get current time
    time_t rawtime;
    time(&rawtime);
    localtime_r(&rawtime, &timeInfo);

    // Initial display update
    updateTimeDisplay();
    updateDateDisplay();
}

void TimeManager::setIndianTime(int year, int month, int day, int hour, int min, int sec)
{
    // Set time in local time (IST)
    struct tm tm;
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;

    // Convert to time_t (UTC)
    time_t t = mktime(&tm);

// Apply Indian timezone offset manually if needed
#ifndef __linux__
    t -= IST_OFFSET; // mktime assumes local time, so we subtract offset to get UTC
#endif

    // Set system time
    timeval tv = {t, 0};
    settimeofday(&tv, nullptr);

    // Update display immediately
    localtime_r(&t, &timeInfo);
    updateTimeDisplay();
    updateDateDisplay();
}

void TimeManager::update()
{
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

// Apply offset manually if timezone not supported
#ifndef __linux__
    rawtime += IST_OFFSET;
#endif

    localtime_r(&rawtime, &timeInfo);

    // Only update display if on homepage
    if (lv_scr_act() == ui_screen_homepage)
    {
        updateTimeDisplay();

        // Update date only when it changes
        static int lastDay = -1;
        if (timeInfo.tm_mday != lastDay)
        {
            updateDateDisplay();
            lastDay = timeInfo.tm_mday;
        }
    }
}

// Rest of the implementation remains the same...
void TimeManager::updateTimeDisplay()
{
    if (!ui_homepage_label_time)
        return;

    char timeStr[9];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d",
             timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

    lv_label_set_text(ui_homepage_label_time, timeStr);
}

void TimeManager::updateDateDisplay()
{
    if (!ui_homepage_label_date)
        return;

    static const char *monthNames[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"};

    char dateStr[20];
    snprintf(dateStr, sizeof(dateStr), "%d %s %d",
             timeInfo.tm_mday, monthNames[timeInfo.tm_mon], 1900 + timeInfo.tm_year);

    lv_label_set_text(ui_homepage_label_date, dateStr);
}