/*
 * clock_data.cpp
 * Tầng đóng gói & điều khiển dữ liệu Smart Clock
 *
 * Không #include WebServer, WiFi hay HTTP bất kỳ.
 * Chỉ thao tác với global_vars, sensor_handler, RTC, GPIO.
 */

#include "clock_data.h"
#include "global_vars.h"
#include <Preferences.h>

// ============================================================================
// JSON BUILDER
// ============================================================================
String clockData_buildStatusJSON()
{
    RTC_DS3231 *rtc = getRTC();
    DateTime now = rtc->now();

    static const char *DAY_NAMES[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char *MODE_NAMES[] = {
        "TEMP_HUMIDITY", "DATE_TIME", "ALARM", "STOPWATCH", "COUNTDOWN"};

    // --- Stopwatch: tính elapsed realtime nếu đang chạy ---
    unsigned long swTime = timerCurrentTime;
    if (isTimerRunning)
        swTime = millis() - timerStartTime;

    // --- Countdown: tính remaining realtime nếu đang chạy ---
    unsigned long cdLeft = countdownRemaining;
    if (isCountdownRunning)
    {
        unsigned long elapsed = millis() - countdownStartTime;
        cdLeft = (elapsed >= countdownDuration) ? 0 : (countdownDuration - elapsed);
    }

    String j;
    j.reserve(512); // tránh realloc nhiều lần
    j = "{";

    // ---- Thời gian thực ----
    j += "\"time\":{";
    j += "\"hour\":" + String(now.hour()) + ",";
    j += "\"minute\":" + String(now.minute()) + ",";
    j += "\"second\":" + String(now.second()) + ",";
    j += "\"day\":" + String(now.day()) + ",";
    j += "\"month\":" + String(now.month()) + ",";
    j += "\"year\":" + String(now.year()) + ",";
    j += "\"weekday\":\"" + String(DAY_NAMES[now.dayOfTheWeek()]) + "\"";
    j += "},";

    // ---- Cảm biến ----
    j += "\"sensor\":{";
    j += "\"temp\":" + String(g_temp, 1) + ",";
    j += "\"humidity\":" + String(g_humi, 1);
    j += "},";

    // ---- Chế độ ----
    j += "\"mode\":" + String(displayMode) + ",";
    j += "\"modeName\":\"" + String(MODE_NAMES[displayMode]) + "\",";

    // ---- Báo thức ----
    j += "\"alarm\":{";
    j += "\"hour\":" + String(alarmHour) + ",";
    j += "\"minute\":" + String(alarmMinute) + ",";
    j += "\"triggered\":" + String(alarmTriggered ? "true" : "false") + ",";
    j += "\"editHour\":" + String(alarmEditHour ? "true" : "false");
    j += "},";

    // ---- Stopwatch ----
    j += "\"stopwatch\":{";
    j += "\"running\":" + String(isTimerRunning ? "true" : "false") + ",";
    j += "\"elapsed\":" + String(swTime) + ",";
    j += "\"lapCount\":" + String(lapCount) + ",";
    j += "\"laps\":[";
    for (int i = 0; i < lapCount; i++)
    {
        if (i)
            j += ",";
        j += String(laps[i]);
    }
    j += "]},";

    // ---- Countdown ----
    j += "\"countdown\":{";
    j += "\"running\":" + String(isCountdownRunning ? "true" : "false") + ",";
    j += "\"editing\":" + String(countdownEditing ? "true" : "false") + ",";
    j += "\"triggered\":" + String(countdownTriggered ? "true" : "false") + ",";
    j += "\"remaining\":" + String(cdLeft) + ",";
    j += "\"duration\":" + String(countdownDuration) + ",";
    j += "\"editHours\":" + String(c_hours) + ",";
    j += "\"editMinutes\":" + String(c_minutes) + ",";
    j += "\"editSeconds\":" + String(c_seconds);
    j += "},";

    // ---- Hệ thống ----
    j += "\"system\":{";
    j += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
    j += "\"uptime\":" + String(millis() / 1000UL);
    j += "}";

    j += "}";
    return j;
}

// ============================================================================
// MODE
// ============================================================================
bool clockData_setMode(int mode)
{
    if (mode < 0 || mode >= TOTAL_MODES)
        return false;

    displayMode = mode;
    getLedControl()->clearDisplay(0);
    getLCD()->clear();

    if (displayMode == MODE_STOPWATCH)
    {
        timerCurrentTime = 0;
        isTimerRunning = false;
    }
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

    Serial.printf("[DATA] Mode -> %d\n", displayMode);
    return true;
}

// ============================================================================
// ALARM
// ============================================================================
bool clockData_setAlarm(int hour, int minute)
{
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
        return false;

    alarmHour = hour;
    alarmMinute = minute;
    alarmTriggered = false;

    // Lưu vào flash để khôi phục sau khi reset
    Preferences prefs;
    prefs.begin("clock", false);
    prefs.putInt("alarmHour", alarmHour);
    prefs.putInt("alarmMinute", alarmMinute);
    prefs.end();

    Serial.printf("[DATA] Alarm -> %02d:%02d\n", hour, minute);
    return true;
}

void clockData_stopAlarm()
{
    alarmTriggered = false;
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    Serial.println("[DATA] Alarm stopped");
}

// ============================================================================
// STOPWATCH
// ============================================================================
void clockData_swStart()
{
    if (!isTimerRunning)
    {
        timerStartTime = millis() - timerCurrentTime;
        isTimerRunning = true;
        Serial.println("[DATA] SW start");
    }
}

void clockData_swStop()
{
    if (isTimerRunning)
    {
        timerCurrentTime = millis() - timerStartTime;
        isTimerRunning = false;
        if (lapCount < 5)
            laps[lapCount++] = timerCurrentTime;
        Serial.printf("[DATA] SW stop, lap %d = %lums\n", lapCount, timerCurrentTime);
    }
}

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
// COUNTDOWN
// ============================================================================
bool clockData_cdSet(int hours, int minutes, int seconds)
{
    hours = constrain(hours, 0, 99);
    minutes = constrain(minutes, 0, 59);
    seconds = constrain(seconds, 0, 59);

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

void clockData_cdStop()
{
    isCountdownRunning = false;
    Serial.println("[DATA] CD stop");
}

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