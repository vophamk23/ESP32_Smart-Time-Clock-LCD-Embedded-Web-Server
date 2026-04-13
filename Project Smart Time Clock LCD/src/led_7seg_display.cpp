#include "led_7seg_display.h"
#include "lcd_display.h"
#include "config.h"
#include "global_vars.h"

static LedControl lc(DIN_PIN, CLK_PIN, CS_PIN, 1);

// ============================================================================
// HÀM KHỞI TẠO LED 7-SEGMENT
// ============================================================================
// Chức năng: Cấu hình ban đầu cho LED 7-segment (bật, độ sáng, xóa màn hình)
void initDisplay()
{
    lc.shutdown(0, false);
    lc.setIntensity(0, DISPLAY_INTENSITY);
    lc.clearDisplay(0);
}

LedControl *getLedControl()
{
    return &lc;
}

// ============================================================================
// CẬP NHẬT: Hiển thị thời gian bấm giờ (lap)
// Tham số: lc - con trỏ tới đối tượng LED 7-segment, t - thời gian bấm giờ (lap) tính bằng ms
// ============================================================================
void displayLapValue(LedControl *lc, unsigned long t)
{
    unsigned long hours = (t / 3600000) % 100;
    unsigned long minutes = (t / 60000) % 60;
    unsigned long seconds = (t / 1000) % 60;
    unsigned long centiseconds = (t / 10) % 100;

    lc->setDigit(0, 7, hours / 10, false);
    lc->setDigit(0, 6, hours % 10, true);
    lc->setDigit(0, 5, minutes / 10, false);
    lc->setDigit(0, 4, minutes % 10, true);
    lc->setDigit(0, 3, seconds / 10, false);
    lc->setDigit(0, 2, seconds % 10, true);
    lc->setDigit(0, 1, centiseconds / 10, false);
    lc->setDigit(0, 0, centiseconds % 10, false);
}

// ============================================================================
// CẬP NHẬT: Hiển thị laps trên cả LED và LCD
// ============================================================================
void showSavedLaps(LedControl *lc)
{
    // State machine tĩnh — giữ trạng thái giữa các lần gọi
    static int s_lapIndex = -1; // -1 = chưa bắt đầu
    static unsigned long s_lapShowTime = 0;

    // ── Khởi động lần đầu ──
    if (s_lapIndex == -1)
    {
        if (lapCount <= 0)
        {
            Serial.println("[LAP] No laps to show");
            getLCD()->clear();
            getLCD()->setCursor(0, 0);
            getLCD()->print("No laps saved");
            return;
        }
        s_lapIndex = 0;
        s_lapShowTime = millis();
        Serial.printf("[LAP] Showing %d laps\n", lapCount);
    }

    // ── Đã hiển thị hết ──
    if (s_lapIndex >= lapCount)
    {
        // Reset tất cả
        for (int i = 0; i < 5; i++)
            laps[i] = 0;
        lapCount = 0;
        timerCurrentTime = 0;
        timerStartTime = 0;
        isTimerRunning = false;
        s_lapIndex = -1;

        lc->clearDisplay(0);
        getLCD()->clear();
        getLCD()->setCursor(0, 0);
        getLCD()->print("Laps cleared");
        Serial.println("[LAP] Laps cleared after show");
        return;
    }

    // ── Hiển thị lap hiện tại ──
    displayLapValue(lc, laps[s_lapIndex]);
    displayLCD_LapValue(laps[s_lapIndex]);

    // Cập nhật header LCD
    getLCD()->setCursor(0, 0);
    getLCD()->print("Lap ");
    getLCD()->print(s_lapIndex + 1);
    getLCD()->print(" of ");
    getLCD()->print(lapCount);
    getLCD()->print("   ");

    // Tiến sang lap tiếp theo sau 2 giây
    if (millis() - s_lapShowTime >= 2000)
    {
        Serial.printf("[LAP] Lap %d shown\n", s_lapIndex + 1);
        s_lapIndex++;
        s_lapShowTime = millis();
    }
}

// ============================================================================
// CẬP NHẬT: Hiển thị nhiệt độ và độ ẩm trên LED 7-segment
// ============================================================================
void displayTempHumidity(DHT *dht, LedControl *lc)
{
    if (g_temp == 0.0f && g_humi == 0.0f)
        return;

    int tempInt = (int)(g_temp * 10);
    lc->setDigit(0, 7, (tempInt / 100), false);
    lc->setDigit(0, 6, (tempInt / 10) % 10, true); // dấu thập phân
    lc->setDigit(0, 5, tempInt % 10, false);
    lc->setChar(0, 4, 'C', false);

    int humiInt = (int)(g_humi * 10);
    lc->setDigit(0, 3, (humiInt / 100), false);
    lc->setDigit(0, 2, (humiInt / 10) % 10, true); // dấu thập phân
    lc->setDigit(0, 1, humiInt % 10, false);
    lc->setChar(0, 0, 'H', false);
}

// ============================================================================
// CẬP NHẬT: Hiển thị thời gian hiện tại, báo thức, bấm giờ và đếm ngược trên LED 7-segment
// ============================================================================
void displayDateTime(RTC_DS3231 *rtc, LedControl *lc)
{
    DateTime now = rtc->now();

    lc->setDigit(0, 7, now.hour() / 10, false);
    lc->setDigit(0, 6, now.hour() % 10, true);
    lc->setDigit(0, 5, now.minute() / 10, false);
    lc->setDigit(0, 4, now.minute() % 10, false);

    lc->setDigit(0, 3, now.day() / 10, false);
    lc->setDigit(0, 2, now.day() % 10, true);
    lc->setDigit(0, 1, now.month() / 10, false);
    lc->setDigit(0, 0, now.month() % 10, false);
}

// ============================================================================
// CẬP NHẬT: Hiển thị báo thức trên LED 7-segment
// ============================================================================
void displayAlarm(RTC_DS3231 *rtc, LedControl *lc)
{
    DateTime now = rtc->now();

    // Hiển thị giờ hiện tại (4 digit trái)
    lc->setDigit(0, 7, now.hour() / 10, false);
    lc->setDigit(0, 6, now.hour() % 10, true);
    lc->setDigit(0, 5, now.minute() / 10, false);
    lc->setDigit(0, 4, now.minute() % 10, false);

    // Hiển thị giờ báo thức (4 digit phải)
    lc->setDigit(0, 3, alarmHour / 10, false);
    lc->setDigit(0, 2, alarmHour % 10, true);
    lc->setDigit(0, 1, alarmMinute / 10, false);
    lc->setDigit(0, 0, alarmMinute % 10, false);
}

// ============================================================================
// CẬP NHẬT: Hiển thị bấm giờ trên LED 7-segment
// ============================================================================
void displayStopwatch(LedControl *lc)
{
    if (isTimerRunning)
    {
        timerCurrentTime = millis() - timerStartTime;
    }

    unsigned long hours = (timerCurrentTime / 3600000) % 100;
    unsigned long minutes = (timerCurrentTime / 60000) % 60;
    unsigned long seconds = (timerCurrentTime / 1000) % 60;
    unsigned long centiseconds = (timerCurrentTime / 10) % 100;

    lc->setDigit(0, 7, hours / 10, false);
    lc->setDigit(0, 6, hours % 10, true);
    lc->setDigit(0, 5, minutes / 10, false);
    lc->setDigit(0, 4, minutes % 10, true);
    lc->setDigit(0, 3, seconds / 10, false);
    lc->setDigit(0, 2, seconds % 10, true);
    lc->setDigit(0, 1, centiseconds / 10, false);
    lc->setDigit(0, 0, centiseconds % 10, false);
}

// ============================================================================
// CẬP NHẬT: Hiển thị đếm ngược trên LED 7-segment, bao gồm cả chế độ chỉnh giờ và trạng thái đang chạy
// ============================================================================
void displayCountdown(LedControl *lc)
{
    if (countdownEditing)
    {
        bool blinkOn = ((millis() / 500) % 2) == 0;

        if (!(countdownEditField == 0 && !blinkOn))
        {
            lc->setDigit(0, 7, (c_hours / 10) % 10, false);
            lc->setDigit(0, 6, c_hours % 10, true);
        }
        else
        {
            lc->setChar(0, 7, ' ', false);
            lc->setChar(0, 6, ' ', false);
        }

        if (!(countdownEditField == 1 && !blinkOn))
        {
            lc->setDigit(0, 5, (c_minutes / 10) % 10, false);
            lc->setDigit(0, 4, c_minutes % 10, true);
        }
        else
        {
            lc->setChar(0, 5, ' ', false);
            lc->setChar(0, 4, ' ', false);
        }

        if (!(countdownEditField == 2 && !blinkOn))
        {
            lc->setDigit(0, 3, (c_seconds / 10) % 10, false);
            lc->setDigit(0, 2, c_seconds % 10, true);
        }
        else
        {
            lc->setChar(0, 3, ' ', false);
            lc->setChar(0, 2, ' ', false);
        }

        lc->setDigit(0, 1, 0, false);
        lc->setDigit(0, 0, 0, false);
    }
    else if (isCountdownRunning)
    {
        unsigned long elapsed = millis() - countdownStartTime;
        if (elapsed >= countdownDuration)
        {
            countdownRemaining = 0;
            isCountdownRunning = false;
            countdownTriggered = true;
            digitalWrite(BUZZER_PIN, HIGH); // Bật buzzer khi hết giờ
            digitalWrite(LED_PIN, HIGH);    // Bật LED báo hiệu
        }
        else
        {
            countdownRemaining = countdownDuration - elapsed;
        }

        unsigned long t = countdownRemaining;
        unsigned long hours = (t / 3600000) % 100;
        unsigned long minutes = (t / 60000) % 60;
        unsigned long seconds = (t / 1000) % 60;
        unsigned long centiseconds = (t / 10) % 100;

        lc->setDigit(0, 7, hours / 10, false);
        lc->setDigit(0, 6, hours % 10, true);
        lc->setDigit(0, 5, minutes / 10, false);
        lc->setDigit(0, 4, minutes % 10, true);
        lc->setDigit(0, 3, seconds / 10, false);
        lc->setDigit(0, 2, seconds % 10, true);
        lc->setDigit(0, 1, centiseconds / 10, false);
        lc->setDigit(0, 0, centiseconds % 10, false);
    }
    else
    {
        if (countdownTriggered)
        {
            lc->setDigit(0, 7, 0, false);
            lc->setDigit(0, 6, 0, true);
            lc->setDigit(0, 5, 0, false);
            lc->setDigit(0, 4, 0, true);
            lc->setDigit(0, 3, 0, false);
            lc->setDigit(0, 2, 0, true);
            lc->setDigit(0, 1, 0, false);
            lc->setDigit(0, 0, 0, false);
        }
        else
        {
            lc->setDigit(0, 7, (c_hours_initial / 10) % 10, false);
            lc->setDigit(0, 6, c_hours_initial % 10, true);
            lc->setDigit(0, 5, (c_minutes_initial / 10) % 10, false);
            lc->setDigit(0, 4, c_minutes_initial % 10, true);
            lc->setDigit(0, 3, (c_seconds_initial / 10) % 10, false);
            lc->setDigit(0, 2, c_seconds_initial % 10, true);
            lc->setDigit(0, 1, 0, false);
            lc->setDigit(0, 0, 0, false);
        }
    }
}