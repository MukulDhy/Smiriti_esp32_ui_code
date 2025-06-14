#ifndef TIME_H
#define TIME_H

#include <Arduino.h>
#include <time.h>
#include <sys/time.h>
#include <lvgl.h>
#include "ui/ui.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class TimeManager
{
public:
    static void initialize();
    static void update();
    static void setIndianTime(int year, int month, int day, int hour, int min, int sec);
    static String getFormattedTime();
    static bool isTimeValid();
    static void forceUpdate();
    static void reset();

private:
    // Thread-safe update functions
    static void updateTimeDisplaySafe();
    static void updateDateDisplaySafe();

    // Legacy functions for compatibility
    static void updateTimeDisplay();
    static void updateDateDisplay();

    static unsigned long lastUpdateTime;
    static struct tm timeInfo;
    static const int IST_OFFSET; // Declaration only - defined in .cpp
    static bool initialized;
    static bool timeValid;
};

#endif // TIME_H