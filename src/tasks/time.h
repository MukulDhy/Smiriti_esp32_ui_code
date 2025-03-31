#ifndef TIME_H
#define TIME_H

#include <RTClib.h>
#include <lvgl.h>
#include "ui/ui.h"

class TimeManager
{
public:
    static bool begin();
    static void update();
    static bool syncWithNTP();

private:
    static RTC_DS3231 rtc;
    static DateTime lastUpdate;
    static bool rtcFound;

    // NTP Configuration
    static const char *NTP_SERVERS[];
    static const long GMT_OFFSET;
    static const int DAYLIGHT_OFFSET;

    static const char *monthName(uint8_t month);
    static void updateTimeDisplay(const DateTime &now);
    static void updateDateDisplay(const DateTime &now);
    static bool tryNTPSync(const char *server);
};

#endif