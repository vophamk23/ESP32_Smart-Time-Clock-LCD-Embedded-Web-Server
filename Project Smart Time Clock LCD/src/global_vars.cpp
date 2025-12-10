#include "global_vars.h"

// ==================== KHỞI TẠO BIẾN ĐIỀU KHIỂN CHUNG ====================
int displayMode = 0;               // Chế độ hiển thị hiện tại (0: Đồng hồ, 1: Bấm giờ, 2: Đếm ngược, 3: Báo thức)
unsigned long lastButtonPress = 0; // Thời điểm lần cuối nhấn nút (dùng để debounce)

// ==================== KHỞI TẠO BIẾN BẤM GIỜ ====================
bool isTimerRunning = false;             // Trạng thái bấm giờ (true: đang chạy, false: dừng)
unsigned long timerStartTime = 0;        // Thời điểm bắt đầu bấm giờ (milliseconds)
unsigned long timerCurrentTime = 0;      // Thời gian hiện tại của bấm giờ (milliseconds)
unsigned long laps[5] = {0, 0, 0, 0, 0}; // Mảng lưu 3 vòng lap gần nhất
int lapCount = 0;                        // Số lượng lap đã ghi (tối đa 3)

// ==================== KHỞI TẠO BIẾN ĐÉM NGƯỢC ====================
bool isCountdownRunning = false;          // Trạng thái đếm ngược (true: đang chạy, false: dừng)
unsigned long countdownStartTime = 0;     // Thời điểm bắt đầu đếm ngược (milliseconds)
unsigned long countdownDuration = 180000; // Tổng thời gian đếm ngược (mặc định 3 phút = 180000ms)
unsigned long countdownRemaining = 0;     // Thời gian còn lại của đếm ngược (milliseconds)
bool countdownEditing = true;             // Chế độ chỉnh sửa thời gian đếm ngược (true: đang chỉnh, false: đã xong)
int countdownEditField = 0;               // Trường đang chỉnh sửa (0: giờ, 1: phút, 2: giây)
int c_hours = 0;                          // Giờ hiện tại của đếm ngược
int c_minutes = 0;                        // Phút hiện tại của đếm ngược
int c_seconds = 0;                        // Giây hiện tại của đếm ngược
int c_hours_initial = 0;                  // Giờ ban đầu đã cài đặt (để reset)
int c_minutes_initial = 0;                // Phút ban đầu đã cài đặt (để reset)
int c_seconds_initial = 0;                // Giây ban đầu đã cài đặt (để reset)
bool countdownTriggered = false;          // Cờ báo đã hết thời gian đếm ngược (true: đã hết giờ)

// ==================== KHỞI TẠO BIẾN BÁO THỨC ====================
int alarmHour = 10;          // Giờ của báo thức (mặc định 5h)
int alarmMinute = 30;        // Phút của báo thức (mặc định 21 phút)
bool alarmTriggered = false; // Cờ báo đã kích hoạt báo thức (true: đang báo)
bool alarmEditHour = true;   // Trường đang chỉnh sửa (true: chỉnh giờ, false: chỉnh phút)

// ==================== KHỞI TẠO BIẾN LED ====================
unsigned long lastLEDToggle = 0; // Thời điểm toggle LED lần cuối
bool ledState = false;           // Trạng thái LED (false = LOW/tắt)