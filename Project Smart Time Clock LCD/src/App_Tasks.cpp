/*
 * app_tasks.cpp - Task Implementation
 */

#include "app_tasks.h"

/* ==================== TASK 1: BUTTON SCANNING ==================== */
// Quét 3 nút nhấn (MODE, SET, INC) mỗi 50ms
void Task_CheckButtons(void)
{
    checkButtons(getLedControl());
}

/* ==================== TASK 2: LED 7-SEGMENT UPDATE ==================== */
// Cập nhật hiển thị LED 7-segment theo chế độ hiện tại - 100ms
void Task_UpdateDisplay(void)
{
    switch (displayMode)
    {
    case MODE_TEMP_HUMIDITY:
        displayTempHumidity(getDHT(), getLedControl());
        break;
    case MODE_DATE_TIME:
        displayDateTime(getRTC(), getLedControl());
        break;
    case MODE_ALARM:
        displayAlarm(getRTC(), getLedControl());
        break;
    case MODE_STOPWATCH:
        displayStopwatch(getLedControl());
        break;
    case MODE_COUNTDOWN:
        displayCountdown(getLedControl());
        break;
    }
}

/* ==================== TASK 3: LCD 16x2 UPDATE ==================== */
// Cập nhật hiển thị LCD song song với LED - 100ms
void Task_UpdateLCD(void)
{
    switch (displayMode)
    {
    case MODE_TEMP_HUMIDITY:
        displayLCD_TempHumidity(getDHT());
        break;
    case MODE_DATE_TIME:
        displayLCD_DateTime(getRTC());
        break;
    case MODE_ALARM:
        displayLCD_Alarm(getRTC());
        break;
    case MODE_STOPWATCH:
        displayLCD_Stopwatch();
        break;
    case MODE_COUNTDOWN:
        displayLCD_Countdown();
        break;
    }
}

/* ==================== TASK 4: ALARM CHECKER ==================== */
// Kiểm tra báo thức mỗi giây, trigger buzzer khi khớp giờ
void Task_CheckAlarm(void)
{
    if (displayMode == MODE_ALARM && !alarmTriggered)
    {
        DateTime now = getRTC()->now();
        if (now.hour() == alarmHour &&
            now.minute() == alarmMinute &&
            now.second() == 0)
        {
            alarmTriggered = true;
            digitalWrite(BUZZER_PIN, HIGH);
            Serial.println("⏰ ALARM TRIGGERED!");
        }
    }
}

/* ==================== TASK 6: LED BLINK HANDLER ==================== */
void Task_HandleLEDBlink()
{
    // Chỉ nhấp nháy khi báo thức hoặc countdown được kích hoạt
    if (alarmTriggered || countdownTriggered)
    {
        // Kiểm tra đã đủ thời gian để toggle LED chưa
        if (millis() - lastLEDToggle >= TASK_LED_INDICATOR_DELAY_MS)
        {
            lastLEDToggle = millis();                     // Cập nhật thời điểm toggle
            ledState = !ledState;                         // Đảo trạng thái LED
            digitalWrite(LED_PIN, ledState ? HIGH : LOW); // Cập nhật LED
        }
    }
    else
    {
        // Khi không có báo động, đảm bảo LED tắt
        digitalWrite(LED_PIN, LOW);
        ledState = false;
    }
}

/* ==================== TASK 5: SENSOR READER ==================== */
// Đọc nhiệt độ & độ ẩm từ DHT11 mỗi 2 giây
void Task_ReadSensors(void)
{
    if (displayMode == MODE_TEMP_HUMIDITY)
    {
        float temp = getDHT()->readTemperature();
        float humi = getDHT()->readHumidity();

        if (!isnan(temp) && !isnan(humi))
        {
            Serial.printf("DHT -> T=%.1f°C, H=%.1f%%\n", temp, humi);
        }
    }
}
