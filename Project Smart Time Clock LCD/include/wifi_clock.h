#ifndef WIFI_CLOCK_H
#define WIFI_CLOCK_H

#include <Arduino.h>
#include <WiFi.h>
#include <RTClib.h>

// ssid/pass truyền vào từ config.h
void initWiFiAndNTP(const char *ssid, const char *pass, RTC_DS3231 *rtc);

#endif