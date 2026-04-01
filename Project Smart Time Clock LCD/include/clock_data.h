#ifndef CLOCK_DATA_H
#define CLOCK_DATA_H

#include <Arduino.h>
#include <Preferences.h>
#include "global_vars.h"
#include "sensor_handler.h"
#include "led_7seg_display.h"
#include "lcd_display.h"
#include "config.h"

// ============================================================================
// clock_data.h
// Tầng đóng gói dữ liệu — tách biệt hoàn toàn khỏi HTTP/WebServer
//
// Trách nhiệm:
//   - Đọc toàn bộ state (RTC, cảm biến, stopwatch, countdown, alarm...)
//   - Đóng gói thành JSON string trả về cho WebServer
//   - Xử lý các lệnh điều khiển (set alarm, start/stop timer...) từ WebServer
//   - Không biết gì về HTTP, WebServer, WiFi
// ============================================================================

// ---------- JSON ----------
String clockData_buildStatusJSON();

// ---------- Mode ----------
bool clockData_setMode(int mode);

// ---------- Alarm ----------
bool clockData_setAlarm(int hour, int minute);
void clockData_stopAlarm();

// ---------- Stopwatch ----------
void clockData_swStart();
void clockData_swStop();
void clockData_swReset();

// ---------- Countdown ----------
bool clockData_cdSet(int hours, int minutes, int seconds);
bool clockData_cdStart();
void clockData_cdStop();
void clockData_cdReset();

// ---------- Helper dùng nội bộ & bởi WebServer ----------
static inline void saveAlarmToFlash()
{
    Preferences prefs;
    prefs.begin("clock", false);
    prefs.putInt("alarmHour", alarmHour);
    prefs.putInt("alarmMinute", alarmMinute);
    prefs.end();
}

#endif // CLOCK_DATA_H