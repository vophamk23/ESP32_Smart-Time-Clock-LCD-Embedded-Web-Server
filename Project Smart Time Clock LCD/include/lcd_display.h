#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>
// #include <DHT.h>
// #include <RTClib.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "DHT.h"
#include "RTClib.h"

// ============================================================================
// HÀM KHỞI TẠO VÀ TRẢ VỀ ĐỐI TƯỢNG LCD
// ============================================================================
void initLCD();
LiquidCrystal_I2C *getLCD();

// ============================================================================
// CÁC HÀM HIỂN THỊ CHO TỪNG CHẾ ĐỘ
// ============================================================================
void displayLCD_TempHumidity(DHT *dht);
void displayLCD_DateTime(RTC_DS3231 *rtc);
void displayLCD_Alarm(RTC_DS3231 *rtc);
void displayLCD_Stopwatch();
void displayLCD_Countdown();

// ============================================================================
// HÀM HIỂN THỊ LAP VÀ HELPER
// ============================================================================
void displayLCD_LapValue(unsigned long t);
void formatTime(char *buffer, int hours, int minutes, int seconds, int centiseconds);

#endif // LCD_DISPLAY_H