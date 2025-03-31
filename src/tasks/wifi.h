// wifi.h
#ifndef WIFI_H
#define WIFI_H

#include <WiFi.h>
#include <ui/ui.h>

extern const char *ssid;
extern const char *password;

void initWiFi();
void handleWiFi();
void triggerWiFiReconnect();

#endif