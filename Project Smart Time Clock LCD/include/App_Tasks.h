/*
 * app_tasks.h - Task Definitions for Smart Clock
 */

#ifndef APP_TASKS_H
#define APP_TASKS_H

#include <Arduino.h>
#include "scheduler.h"
#include "config.h"
#include "global_vars.h"
#include "led_7seg_display.h"
#include "lcd_display.h"
#include "button_handler.h"
#include "sensor_handler.h"

/* ==================== TIMING CONSTANTS (MS) ==================== */
#define TASK_BUTTON_CHECK_DELAY_MS 50
#define TASK_LED_DISPLAY_DELAY_MS 100
#define TASK_LCD_DISPLAY_DELAY_MS 100
#define TASK_ALARM_CHECK_DELAY_MS 1000
#define TASK_SENSOR_READ_DELAY_MS 2000
#define TASK_HEARTBEAT_DELAY_MS 1000
#define TASK_DEBUG_INFO_DELAY_MS 5000
#define TASK_LED_INDICATOR_DELAY_MS 100

/* ==================== TIMING IN TICKS ==================== */
#define TASK_BUTTON_CHECK_TICKS (TASK_BUTTON_CHECK_DELAY_MS / TIMER_TICK_MS)
#define TASK_LED_DISPLAY_TICKS (TASK_LED_DISPLAY_DELAY_MS / TIMER_TICK_MS)
#define TASK_LCD_DISPLAY_TICKS (TASK_LCD_DISPLAY_DELAY_MS / TIMER_TICK_MS)
#define TASK_ALARM_CHECK_TICKS (TASK_ALARM_CHECK_DELAY_MS / TIMER_TICK_MS)
#define TASK_SENSOR_READ_TICKS (TASK_SENSOR_READ_DELAY_MS / TIMER_TICK_MS)
#define TASK_HEARTBEAT_TICKS (TASK_HEARTBEAT_DELAY_MS / TIMER_TICK_MS)
#define TASK_DEBUG_INFO_TICKS (TASK_DEBUG_INFO_DELAY_MS / TIMER_TICK_MS)
#define TASK_LED_INDICATOR_TICKS (TASK_LED_INDICATOR_DELAY_MS / TIMER_TICK_MS)

/* ==================== TASK FUNCTIONS ==================== */
void Task_CheckButtons(void);
void Task_UpdateDisplay(void);
void Task_UpdateLCD(void);
void Task_CheckAlarm(void);
void Task_HandleLEDBlink(void);
void Task_ReadSensors(void);
void Task_Heartbeat(void);
void Task_DebugInfo(void);

#endif // APP_TASKS_H