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
#include "sensor_handler.h"
#include "App_Tasks.h"

/* ==================== SETUP ==================== */
void setup()
{
  Serial.begin(115200);
  delay(100); // Đợi Serial khởi động

  // ========================================================================
  // 1: KHỞI TẠO PHẦN CỨNG
  // ========================================================================
  initButtons();
  initDisplay();
  initLCD();
  initSensors();

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
}

/* ==================== MAIN LOOP ==================== */

void loop()
{
  // Chỉ gọi Dispatch - mọi thứ được quản lý bởi scheduler!
  SCH_Dispatch_Tasks();

  // Có thể thêm các task không phụ thuộc thời gian ở đây
  // VD: Xử lý Serial commands, WiFi, v.v...

  // Yield cho watchdog timer (quan trọng!)
  yield();
}