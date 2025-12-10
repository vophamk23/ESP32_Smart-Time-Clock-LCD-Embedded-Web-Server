#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

#include <Arduino.h>

// ==================== BIẾN ĐIỀU KHIỂN CHUNG ====================
extern int displayMode;               // Chế độ hiển thị hiện tại
extern unsigned long lastButtonPress; // Thời gian nhấn nút cuối cùng

// ==================== BIẾN BẤM GIỜ (STOPWATCH) ====================
extern bool isTimerRunning;            // Đang chạy bấm giờ?
extern unsigned long timerStartTime;   // Thời điểm bắt đầu
extern unsigned long timerCurrentTime; // Thời gian hiện tại
extern unsigned long laps[5];          // Lưu 5 lap
extern int lapCount;                   // Số lap đã lưu

// ==================== BIẾN ĐẾM NGƯỢC (COUNTDOWN) ====================
extern bool isCountdownRunning;          // Đang chạy đếm ngược?
extern unsigned long countdownStartTime; // Thời điểm bắt đầu
extern unsigned long countdownDuration;  // Tổng thời gian đếm ngược
extern unsigned long countdownRemaining; // Thời gian còn lại
extern bool countdownEditing;            // Đang ở chế độ chỉnh?
extern int countdownEditField;           // Field đang chỉnh (0=h, 1=m, 2=s)
extern int c_hours;                      // Giờ đếm ngược
extern int c_minutes;                    // Phút đếm ngược
extern int c_seconds;                    // Giây đếm ngược
extern int c_hours_initial;              // Giá trị giờ ban đầu
extern int c_minutes_initial;            // Giá trị phút ban đầu
extern int c_seconds_initial;            // Giá trị giây ban đầu
extern bool countdownTriggered;          // Đã kích hoạt buzzer?

// ==================== BIẾN BÁO THỨC (ALARM) ====================
extern int alarmHour;       // Giờ báo thức
extern int alarmMinute;     // Phút báo thức
extern bool alarmTriggered; // Báo thức đã kích hoạt?
extern bool alarmEditHour;  // Đang chỉnh giờ hay phút?

// ==================== BIẾN ĐIỀU KHIỂN LED ====================
extern unsigned long lastLEDToggle; // Thời điểm toggle LED lần cuối
extern bool ledState;               // Trạng thái hiện tại của LED (HIGH/LOW)
#endif // GLOBAL_VARS_H