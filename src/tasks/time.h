#ifndef TIME_H
#define TIME_H

#include <lvgl.h>
#include "ui/ui.h"
#include <Arduino.h>

class TimeManager
{
public:
    static void initialize();
    static void update();
    static void setIndianTime(int year, int month, int day, int hour, int min, int sec);
    static String getFormattedTime(); // Added this line

private:
    static void updateTimeDisplay();
    static void updateDateDisplay();
    static void applyTimeOffset();

    static unsigned long lastUpdateTime;
    static struct tm timeInfo;
    static const int IST_OFFSET = 5 * 3600 + 30 * 60; // 5 hours 30 minutes in seconds
};

#endif