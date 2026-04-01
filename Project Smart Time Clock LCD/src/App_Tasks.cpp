/*
 * app_tasks.cpp - Task Implementation
 */

#include "app_tasks.h"
#include "clock_webserver.h"

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
void Task_ReadSensors(void)
{
    float temp = getDHT()->readTemperature();
    float humi = getDHT()->readHumidity();

    if (!isnan(temp) && !isnan(humi))
    {
        g_temp = temp;
        g_humi = humi;
    }
}

/* ==================== TASK 7: SERIAL MONITOR ==================== */
void Task_SerialMonitor(void)
{
    DateTime now = getRTC()->now();

    // Header chung
    Serial.println("========== SMART CLOCK ==========");
    Serial.printf("  Time : %02d:%02d:%02d\n",
                  now.hour(), now.minute(), now.second());
    Serial.printf("  Date : %02d/%02d/%04d\n",
                  now.day(), now.month(), now.year());

    const char *modeStr[] = {
        "TEMP/HUMI", "DATE/TIME", "ALARM", "STOPWATCH", "COUNTDOWN"};
    Serial.printf("  Mode : %s\n", modeStr[displayMode]);
    Serial.println("---------------------------------");

    // Nội dung theo mode
    switch (displayMode)
    {
    case MODE_TEMP_HUMIDITY:
        if (g_temp > 0.0f || g_humi > 0.0f)
        {
            Serial.printf("  Temp : %.1f C\n", g_temp);
            Serial.printf("  Humi : %.0f%%\n", g_humi);
        }
        else
            Serial.println("  Sensor: DHT ERROR");
        break;

    case MODE_DATE_TIME:
        Serial.printf("  Day  : %s\n",
                      (const char *[]){"Sun", "Mon", "Tue", "Wed",
                                       "Thu", "Fri", "Sat"}[now.dayOfTheWeek()]);
        break;

    case MODE_ALARM:
        Serial.printf("  Set  : %02d:%02d\n", alarmHour, alarmMinute);
        Serial.printf("  State: %s\n",
                      alarmTriggered ? "TRIGGERED!" : "Waiting");
        break;

    case MODE_STOPWATCH:
        Serial.printf("  State: %s\n",
                      isTimerRunning ? "RUNNING" : "STOPPED");
        Serial.printf("  Time : %02luh %02lum %02lus\n",
                      (timerCurrentTime / 3600000) % 100,
                      (timerCurrentTime / 60000) % 60,
                      (timerCurrentTime / 1000) % 60);
        Serial.printf("  Laps : %d/%d\n", lapCount, 5);
        for (int i = 0; i < lapCount; i++)
            Serial.printf("    Lap%d: %02lum%02lus%03lu\n",
                          i + 1,
                          (laps[i] / 60000) % 60,
                          (laps[i] / 1000) % 60,
                          (laps[i] / 10) % 100);
        break;

    case MODE_COUNTDOWN:
        if (countdownEditing)
        {
            Serial.printf("  Edit : %02d:%02d:%02d\n",
                          c_hours, c_minutes, c_seconds);
            Serial.printf("  Field: %s\n",
                          countdownEditField == 0 ? "HOUR" : countdownEditField == 1 ? "MIN"
                                                                                     : "SEC");
        }
        else if (isCountdownRunning)
        {
            Serial.printf("  Left : %02luh %02lum %02lus\n",
                          (countdownRemaining / 3600000) % 100,
                          (countdownRemaining / 60000) % 60,
                          (countdownRemaining / 1000) % 60);
            Serial.printf("  Total: %02luh %02lum %02lus\n",
                          (countdownDuration / 3600000) % 100,
                          (countdownDuration / 60000) % 60,
                          (countdownDuration / 1000) % 60);
        }
        else
            Serial.printf("  State: %s\n",
                          countdownTriggered ? "FINISHED!" : "READY");
        break;
    }

    // Màn thống kê — luôn hiển thị ở cuối mỗi lần print
    Serial.println("---------------------------------");
    Serial.printf("  Temp : %.1fC  Humi: %.0f%%\n", g_temp, g_humi);
    Serial.printf("  Alarm: %02d:%02d [%s]\n",
                  alarmHour, alarmMinute,
                  alarmTriggered ? "ON" : "OFF");
    Serial.printf("  SW   : %s  CD: %s\n",
                  isTimerRunning ? "RUN" : "STOP",
                  isCountdownRunning ? "RUN" : "STOP");
    Serial.printf("  Heap : %u bytes\n", ESP.getFreeHeap());
    Serial.println("=================================");
}

/* ==================== TASK 8: WEBSERVER HANDLER ==================== */
// Xử lý HTTP request từ trình duyệt — poll mỗi 50ms
void Task_WebServer_Handler(void)
{
    Task_WebServer();
}