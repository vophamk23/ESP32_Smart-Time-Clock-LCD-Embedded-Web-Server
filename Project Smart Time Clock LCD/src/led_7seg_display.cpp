#include "led_7seg_display.h"
#include "lcd_display.h"
#include "config.h"
#include "global_vars.h"

static LedControl lc(DIN_PIN, CLK_PIN, CS_PIN, 1);

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
    if (lapCount <= 0)
    {
        Serial.println("No laps to show");
        getLCD()->clear();
        getLCD()->setCursor(0, 0);
        getLCD()->print("No laps saved");
        delay(1500);
        timerCurrentTime = 0;
        timerStartTime = 0;
        isTimerRunning = false;
        return;
    }

    Serial.print("Showing ");
    Serial.print(lapCount);
    Serial.println(" saved laps");

    for (int i = 0; i < lapCount; ++i)
    {
        Serial.print("Lap ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(laps[i]);

        // Hiển thị số lap trên LCD
        getLCD()->clear();
        getLCD()->setCursor(0, 0);
        getLCD()->print("Lap ");
        getLCD()->print(i + 1);
        getLCD()->print(" of ");
        getLCD()->print(lapCount);

        unsigned long start = millis();
        while (millis() - start < 2000)
        {
            displayLapValue(lc, laps[i]);
            displayLCD_LapValue(laps[i]);
            delay(1000);
        }
        lc->clearDisplay(0);
    }

    // Reset after showing
    for (int i = 0; i < 5; ++i)
        laps[i] = 0;
    lapCount = 0;
    timerCurrentTime = 0;
    timerStartTime = 0;
    isTimerRunning = false;

    getLCD()->clear();
    getLCD()->setCursor(0, 0);
    getLCD()->print("Laps cleared");
    delay(1000);

    Serial.println("Laps cleared after show");
}

void displayTempHumidity(DHT *dht, LedControl *lc)
{
    float temp = dht->readTemperature();
    float humi = dht->readHumidity();

    if (isnan(temp) || isnan(humi))
    {
        Serial.println("DHT read failed");
        return;
    }

    Serial.print("DHT -> T=");
    Serial.print(temp);
    Serial.print(" C, H=");
    Serial.print(humi);
    Serial.println(" %");

    if (!isnan(temp) && !isnan(humi))
    {
        int tempInt = (int)(temp * 10);
        lc->setDigit(0, 5, tempInt % 10, false);
        lc->setDigit(0, 6, (tempInt / 10) % 10, true);
        lc->setDigit(0, 7, (tempInt / 100), false);
        lc->setChar(0, 4, 'C', false);

        int humiInt = (int)(humi * 10);
        lc->setDigit(0, 1, humiInt % 10, false);
        lc->setDigit(0, 2, (humiInt / 10) % 10, true);
        lc->setDigit(0, 3, (humiInt / 100), false);
        lc->setChar(0, 0, 'H', false);
    }
}

void displayDateTime(RTC_DS3231 *rtc, LedControl *lc)
{
    DateTime now = rtc->now();

    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.println(now.second());
    Serial.print(now.day());
    Serial.print("/");
    Serial.println(now.month());

    lc->setDigit(0, 7, now.hour() / 10, false);
    lc->setDigit(0, 6, now.hour() % 10, true);
    lc->setDigit(0, 5, now.minute() / 10, false);
    lc->setDigit(0, 4, now.minute() % 10, false);

    lc->setDigit(0, 3, now.day() / 10, false);
    lc->setDigit(0, 2, now.day() % 10, true);
    lc->setDigit(0, 1, now.month() / 10, false);
    lc->setDigit(0, 0, now.month() % 10, false);
}

void displayAlarm(RTC_DS3231 *rtc, LedControl *lc)
{
    DateTime now = rtc->now();

    if (!alarmTriggered && now.hour() == alarmHour &&
        now.minute() == alarmMinute && now.second() == 0)
    {
        alarmTriggered = true;
        Serial.println("Alarm triggered!");
        digitalWrite(BUZZER_PIN, HIGH); // Bật buzzer khi báo thức
        digitalWrite(LED_PIN, HIGH);    // Bật LED báo hiệu
    }

    lc->setDigit(0, 7, now.hour() / 10, false);
    lc->setDigit(0, 6, now.hour() % 10, true);
    lc->setDigit(0, 5, now.minute() / 10, false);
    lc->setDigit(0, 4, now.minute() % 10, false);

    lc->setDigit(0, 3, alarmHour / 10, false);
    lc->setDigit(0, 2, alarmHour % 10, true);
    lc->setDigit(0, 1, alarmMinute / 10, false);
    lc->setDigit(0, 0, alarmMinute % 10, false);
}

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