/*
 * main.cpp
 * Smart Clock với Cooperative Scheduler
 */

#include <Arduino.h>
#include "config.h"
#include "global_vars.h"
#include "scheduler.h"
#include "led_7seg_display.h"
#include "lcd_display.h"
#include "button_handler.h"
#include "wifi_clock.h"
#include "sensor_handler.h"
#include "App_Tasks.h"
#include "Preferences.h"

/* ==================== SETUP ==================== */
void setup()
{
  Serial.begin(115200);
  delay(100); // Đợi Serial khởi động

  // ========================================================================
  // 1: KHỞI TẠO PHẦN CỨNG
  // ========================================================================
  initButtons(); // Nút nhấn
  initDisplay(); // LED 7-segment
  initSensors(); // RTC begin() + lostPower check
  initLCD();     // LCD I2C

  // WiFi connect → NTP sync → RTC adjust → WiFi OFF
  // Nếu không có WiFi hoặc timeout, RTC giữ giờ cũ — hoạt động bình thường
  initWiFiAndNTP(WIFI_SSID, WIFI_PASS, getRTC());

  // ========================================================================
  // 2: KHỞI TẠO SCHEDULER
  // ========================================================================
  SCH_Init();

  // ========================================================================
  // 3: KHỞI ĐỘNG TIMER INTERRUPT
  // ========================================================================
  SCH_Init_Timer();
  Serial.println("✓ Timer started (10ms tick)");

  // ========================================================================
  // 4: THÊM CÁC TASK (DELAY, PERIOD tính bằng TICKS, 1 tick = 10ms)
  // ========================================================================
  SCH_Add_Task(Task_CheckButtons, 0, TASK_BUTTON_CHECK_TICKS);

  SCH_Add_Task(Task_UpdateDisplay, 0, TASK_LED_DISPLAY_TICKS);

  SCH_Add_Task(Task_UpdateLCD, 0, TASK_LCD_DISPLAY_TICKS);

  SCH_Add_Task(Task_CheckAlarm, 0, TASK_ALARM_CHECK_TICKS);

  SCH_Add_Task(Task_HandleLEDBlink, 0, TASK_LED_INDICATOR_TICKS);

  SCH_Add_Task(Task_ReadSensors, 0, TASK_SENSOR_READ_TICKS);

  SCH_Add_Task(Task_SerialMonitor, 0, TASK_SERIAL_MONITOR_TICKS);
}

/* ==================== MAIN LOOP ==================== */

void loop()
{
  SCH_Dispatch_Tasks();

  if (Serial.available())
  {
    char cmd = Serial.read();
    switch (cmd)
    {
    case 'm':
    case 'M':
      displayMode = (displayMode + 1) % TOTAL_MODES;
      getLCD()->clear();
      getLedControl()->clearDisplay(0);
      Serial.printf("[CMD] Mode -> %d (%s)\n", displayMode,
                    (const char *[]){"TEMP/HUMI", "DATE/TIME",
                                     "ALARM", "STOPWATCH", "COUNTDOWN"}[displayMode]);
      break;
    case 'r':
    case 'R':
    {
      Preferences prefs;
      prefs.begin("clock", false);
      prefs.clear(); // xóa toàn bộ
      prefs.end();
      alarmHour = 10;
      alarmMinute = 30;
      Serial.println("[ALARM] Reset to default 10:30");
      break;
    }
    case '0':
      displayMode = MODE_TEMP_HUMIDITY;
      Serial.println("[CMD] Mode -> TEMP/HUMI");
      break;
    case '1':
      displayMode = MODE_DATE_TIME;
      Serial.println("[CMD] Mode -> DATE/TIME");
      break;
    case '2':
      displayMode = MODE_ALARM;
      Serial.println("[CMD] Mode -> ALARM");
      break;
    case '3':
      displayMode = MODE_STOPWATCH;
      timerCurrentTime = 0;
      isTimerRunning = false;
      Serial.println("[CMD] Mode -> STOPWATCH");
      break;
    case '4':
      displayMode = MODE_COUNTDOWN;
      countdownEditing = true;
      isCountdownRunning = false;
      countdownTriggered = false;
      countdownEditField = 0;
      Serial.println("[CMD] Mode -> COUNTDOWN");
      break;

    case '?':
      Serial.println("Commands:");
      Serial.println("  m/M    : next mode");
      Serial.println("  0-4    : switch to mode");
      Serial.println("  r/R    : reset alarm to default 10:30");
      Serial.println("  0      : TEMP/HUMI");
      Serial.println("  1      : DATE/TIME");
      Serial.println("  2      : ALARM");
      Serial.println("  3      : STOPWATCH");
      Serial.println("  4      : COUNTDOWN");
      Serial.println("  ?      : help");
      break;

    default:
      break;
    }

    getLCD()->clear();
    getLedControl()->clearDisplay(0);
  }

  yield();
}