/*
 * clock_data.cpp
 * Tầng đóng gói & điều khiển dữ liệu Smart Clock
 *
 * Vai trò:
 *   - Cung cấp API để webserver và display thao tác với dữ liệu đồng hồ
 *   - Toàn bộ logic: mode, alarm, stopwatch, countdown đều nằm ở đây
 *   - KHÔNG có code HTTP, KHÔNG gọi trực tiếp vào hardware display
 *   - Chỉ thao tác: global_vars, RTC, GPIO (buzzer, LED)
 */

#include "clock_data.h"
#include "global_vars.h"
#include <Preferences.h>

// ============================================================================
// JSON BUILDER — Đóng gói toàn bộ trạng thái đồng hồ thành chuỗi JSON
// ============================================================================
// Trả về JSON string gồm các nhóm:
//   time       → giờ/phút/giây/ngày/tháng/năm/thứ từ RTC
//   sensor     → nhiệt độ và độ ẩm từ DHT
//   mode       → chế độ hiển thị hiện tại (số và tên)
//   alarm      → giờ báo thức, trạng thái triggered, trường đang chỉnh
//   stopwatch  → đang chạy, thời gian đã trôi, danh sách laps
//   countdown  → đang chạy, đang chỉnh, thời gian còn lại, tổng thời gian
//   system     → heap còn trống, uptime tính bằng giây
//
// Dùng snprintf vào char buffer thay vì String concatenation
// → 1 lần allocate heap duy nhất, không bị fragment bộ nhớ
String clockData_buildStatusJSON()
{
    RTC_DS3231 *rtc = getRTC();
    DateTime now = rtc->now();

    static const char *DAY_NAMES[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char *MODE_NAMES[] = {
        "TEMP_HUMIDITY", "DATE_TIME", "ALARM", "STOPWATCH", "COUNTDOWN"};

    // Tính thời gian stopwatch realtime nếu đang chạy
    unsigned long swTime = timerCurrentTime;
    if (isTimerRunning)
        swTime = millis() - timerStartTime;

    // Tính thời gian countdown còn lại realtime nếu đang chạy
    unsigned long cdLeft = countdownRemaining;
    if (isCountdownRunning)
    {
        unsigned long elapsed = millis() - countdownStartTime;
        cdLeft = (elapsed >= countdownDuration) ? 0 : (countdownDuration - elapsed);
    }

    // Build phần laps thành chuỗi riêng vì số lượng lap thay đổi động
    char lapsBuf[64] = "[";
    for (int i = 0; i < lapCount; i++)
    {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "%s%lu", i ? "," : "", laps[i]);
        strncat(lapsBuf, tmp, sizeof(lapsBuf) - strlen(lapsBuf) - 1);
    }
    strncat(lapsBuf, "]", sizeof(lapsBuf) - strlen(lapsBuf) - 1);

    // Build toàn bộ JSON bằng snprintf — 1 lần allocate duy nhất
    char buf[700];
    snprintf(buf, sizeof(buf),
             "{"
             "\"time\":{" // giờ/phút/giây/ngày/tháng/năm/thứ
             "\"hour\":%d,\"minute\":%d,\"second\":%d,"
             "\"day\":%d,\"month\":%d,\"year\":%d,"
             "\"weekday\":\"%s\""
             "},"
             "\"sensor\":{" // nhiệt độ và độ ẩm
             "\"temp\":%.1f,\"humidity\":%.1f"
             "},"
             "\"mode\":%d," // chế độ hiển thị hiện tại (số)
             "\"modeName\":\"%s\","
             "\"alarm\":{" // giờ báo thức, trạng thái báo thức, trường đang chỉnh
             "\"hour\":%d,\"minute\":%d,"
             "\"triggered\":%s,\"editHour\":%s"
             "},"
             "\"stopwatch\":{" // đang chạy, thời gian đã trôi, danh sách laps
             "\"running\":%s,\"elapsed\":%lu,"
             "\"lapCount\":%d,\"laps\":%s"
             "},"
             "\"countdown\":{" // đang chạy, đang chỉnh, thời gian còn lại, tổng thời gian
             "\"running\":%s,\"editing\":%s,\"triggered\":%s,"
             "\"remaining\":%lu,\"duration\":%lu,"
             "\"editHours\":%d,\"editMinutes\":%d,\"editSeconds\":%d"
             "},"
             "\"system\":{" // heap còn trống, uptime tính bằng giây
             "\"heap\":%u,\"uptime\":%lu"
             "}"
             "}",
             // time
             now.hour(), now.minute(), now.second(),
             now.day(), now.month(), now.year(),
             DAY_NAMES[now.dayOfTheWeek()],
             // sensor
             g_temp, g_humi,
             // mode
             displayMode,
             MODE_NAMES[displayMode],
             // alarm
             alarmHour, alarmMinute,
             alarmTriggered ? "true" : "false",
             alarmEditHour ? "true" : "false",
             // stopwatch
             isTimerRunning ? "true" : "false",
             swTime,
             lapCount,
             lapsBuf,
             // countdown
             isCountdownRunning ? "true" : "false",
             countdownEditing ? "true" : "false",
             countdownTriggered ? "true" : "false",
             cdLeft, countdownDuration,
             c_hours, c_minutes, c_seconds,
             // system
             (unsigned int)ESP.getFreeHeap(),
             millis() / 1000UL);

    return String(buf);
}

// ============================================================================
// MODE — Chuyển đổi chế độ hiển thị
// ============================================================================
// Validate mode hợp lệ (0 → TOTAL_MODES-1), cập nhật displayMode
// Reset state tương ứng khi vào stopwatch hoặc countdown
// Trả về true nếu thành công, false nếu mode không hợp lệ
//
// Không gọi clearDisplay/lcd.clear ở đây — caller tự xử lý phần display
bool clockData_setMode(int mode)
{
    if (mode < 0 || mode >= TOTAL_MODES)
        return false;

    displayMode = mode;

    // Reset state khi vào stopwatch để bắt đầu từ 0
    if (displayMode == MODE_STOPWATCH)
    {
        timerCurrentTime = 0;
        isTimerRunning = false;
    }
    // Reset state khi vào countdown để về chế độ chỉnh sửa
    else if (displayMode == MODE_COUNTDOWN)
    {
        countdownEditing = true;
        isCountdownRunning = false;
        countdownTriggered = false;
        countdownEditField = 0;
        c_hours = c_hours_initial;
        c_minutes = c_minutes_initial;
        c_seconds = c_seconds_initial;
    }

    Serial.printf("[DATA] Mode -> %d (%s)\n", displayMode,
                  (const char *[]){"TEMP_HUMIDITY", "DATE_TIME", "ALARM",
                                   "STOPWATCH", "COUNTDOWN"}[displayMode]);
    return true;
}

// ============================================================================
// ALARM — Cài đặt giờ báo thức
// ============================================================================
// Validate giờ (0-23) và phút (0-59)
// Lưu vào Preferences (flash) để giữ sau khi mất điện
// Reset alarmTriggered để tránh báo thức ngay lập tức
// Trả về true nếu hợp lệ, false nếu ngoài khoảng cho phép
bool clockData_setAlarm(int hour, int minute)
{
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
        return false;

    alarmHour = hour;
    alarmMinute = minute;
    alarmTriggered = false;

    // Lưu vào flash để khôi phục sau khi reset nguồn
    Preferences prefs;
    prefs.begin("clock", false);
    prefs.putInt("alarmHour", alarmHour);
    prefs.putInt("alarmMinute", alarmMinute);
    prefs.end();

    Serial.printf("[DATA] Alarm set -> %02d:%02d\n", hour, minute);
    return true;
}

// ============================================================================
// ALARM — Tắt báo thức đang kêu
// ============================================================================
// Tắt buzzer, tắt LED báo hiệu, reset cờ alarmTriggered
void clockData_stopAlarm()
{
    alarmTriggered = false;
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    Serial.println("[DATA] Alarm stopped");
}

// ============================================================================
// STOPWATCH — Bắt đầu hoặc tiếp tục bấm giờ
// ============================================================================
// Tính lại timerStartTime dựa trên thời gian đã chạy trước đó
// → cho phép pause/resume mà không mất thời gian đã tích lũy
// Không làm gì nếu đã đang chạy
void clockData_swStart()
{
    if (!isTimerRunning)
    {
        timerStartTime = millis() - timerCurrentTime;
        isTimerRunning = true;
        Serial.println("[DATA] SW start");
    }
}

// ============================================================================
// STOPWATCH — Dừng và lưu lap
// ============================================================================
// Chụp snapshot thời gian tại thời điểm dừng
// Lưu vào mảng laps[] tối đa 5 laps, bỏ qua nếu đã đầy
// Không làm gì nếu đang dừng
void clockData_swStop()
{
    if (isTimerRunning)
    {
        timerCurrentTime = millis() - timerStartTime;
        isTimerRunning = false;

        if (lapCount < 5)
            laps[lapCount++] = timerCurrentTime;
        else
            Serial.println("[DATA] SW lap storage full");

        Serial.printf("[DATA] SW stop, lap %d = %lums\n", lapCount, timerCurrentTime);
    }
}

// ============================================================================
// STOPWATCH — Reset toàn bộ về 0
// ============================================================================
// Dừng stopwatch, xóa thời gian hiện tại, xóa toàn bộ laps đã lưu
void clockData_swReset()
{
    isTimerRunning = false;
    timerCurrentTime = 0;
    timerStartTime = 0;
    lapCount = 0;
    for (int i = 0; i < 5; i++)
        laps[i] = 0;
    Serial.println("[DATA] SW reset");
}

// ============================================================================
// COUNTDOWN — Cài đặt thời gian đếm ngược
// ============================================================================
// Constrain giá trị về khoảng hợp lệ (hours 0-99, minutes/seconds 0-59)
// Lưu cả giá trị ban đầu (initial) để có thể reset về sau
// Chuyển về chế độ editing, dừng countdown nếu đang chạy
// Trả về true khi thành công
bool clockData_cdSet(int hours, int minutes, int seconds)
{
    hours = constrain(hours, 0, 99);
    minutes = constrain(minutes, 0, 59);
    seconds = constrain(seconds, 0, 59);

    // Lưu giá trị hiện tại và giá trị ban đầu (để reset về)
    c_hours = c_hours_initial = hours;
    c_minutes = c_minutes_initial = minutes;
    c_seconds = c_seconds_initial = seconds;

    countdownEditing = true;
    isCountdownRunning = false;
    countdownTriggered = false;
    countdownEditField = 0;

    Serial.printf("[DATA] CD set -> %02d:%02d:%02d\n", hours, minutes, seconds);
    return true;
}

// ============================================================================
// COUNTDOWN — Bắt đầu đếm ngược
// ============================================================================
// Tính tổng thời gian từ c_hours/c_minutes/c_seconds ra milliseconds
// Ghi nhớ thời điểm bắt đầu để tính thời gian còn lại realtime
// Trả về false nếu tổng thời gian = 0 (chưa cài hoặc cài sai)
bool clockData_cdStart()
{
    unsigned long total = (unsigned long)c_hours * 3600UL + (unsigned long)c_minutes * 60UL + (unsigned long)c_seconds;
    if (total == 0)
        return false;

    countdownDuration = total * 1000UL;
    countdownStartTime = millis();
    isCountdownRunning = true;
    countdownEditing = false;
    countdownTriggered = false;

    Serial.printf("[DATA] CD start %lums\n", countdownDuration);
    return true;
}

// ============================================================================
// COUNTDOWN — Dừng đếm ngược (pause)
// ============================================================================
// Chỉ dừng isCountdownRunning, giữ nguyên thời gian còn lại
// Caller tự xử lý buzzer/LED nếu cần (VD: handleCDStop trong webserver)
void clockData_cdStop()
{
    isCountdownRunning = false;
    Serial.println("[DATA] CD stop");
}

// ============================================================================
// COUNTDOWN — Reset toàn bộ về trạng thái ban đầu
// ============================================================================
// Dừng countdown, tắt buzzer và LED, xóa toàn bộ thời gian
// Quay về chế độ editing để người dùng cài lại từ đầu
void clockData_cdReset()
{
    isCountdownRunning = false;
    countdownTriggered = false;
    countdownEditing = true;
    countdownEditField = 0;
    countdownRemaining = 0;
    countdownDuration = 0;

    c_hours = c_minutes = c_seconds = 0;
    c_hours_initial = c_minutes_initial = c_seconds_initial = 0;

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    Serial.println("[DATA] CD reset");
}