#ifndef _LED_7SEG_DISPLAY_H_
#define _LED_7SEG_DISPLAY_H_

// #include <LedControl.h>
// #include <DHT.h>
// #include <RTClib.h>

#include "LedControl.h"
#include "DHT.h"
#include "RTClib.h"

// ==================== KHỞI TẠO DISPLAY ====================
void initDisplay();
LedControl *getLedControl();

// ==================== HÀM HIỂN THỊ CƠ BẢN ====================
void displayTempHumidity(DHT *dht, LedControl *lc);
void displayDateTime(RTC_DS3231 *rtc, LedControl *lc);
void displayAlarm(RTC_DS3231 *rtc, LedControl *lc);
void displayStopwatch(LedControl *lc);
void displayCountdown(LedControl *lc);

// ==================== HÀM HỖ TRỢ ====================
void displayLapValue(LedControl *lc, unsigned long t);
void showSavedLaps(LedControl *lc);

#endif // _LED_7SEG_DISPLAY_H_