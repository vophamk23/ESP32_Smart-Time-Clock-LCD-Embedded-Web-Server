#include "sensor_handler.h"
#include "config.h"
#include <Arduino.h>
#include <Preferences.h>
#include "global_vars.h"

// Khởi tạo đối tượng cảm biến DHT11 với chân DHT_PIN và loại DHT11
static DHT dht(DHT_PIN, DHT11);

// Khởi tạo đối tượng RTC DS3231 (module đồng hồ thời gian thực)
static RTC_DS3231 rtc;

// Hàm khởi tạo các cảm biến
void initSensors()
{
    dht.begin();
    Serial.println("[SENSOR] DHT initialized");

    if (!rtc.begin())
    {
        Serial.println("[SENSOR] RTC not found!");
        while (1)
            ;
    }

    // Chỉ fallback compile time nếu RTC mất nguồn
    // NTP sẽ ghi đè đúng giờ sau trong setup()
    if (rtc.lostPower())
    {
        Serial.println("[RTC] Lost power - fallback to compile time");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Load alarm từ Flash
    Preferences prefs;
    prefs.begin("clock", true); // true = read only
    alarmHour = prefs.getInt("alarmHour", alarmHour); // Nếu không có giá trị nào được lưu trước đó, sử dụng giá trị hiện tại của alarmHour (mặc định là 7)
    alarmMinute = prefs.getInt("alarmMinute", alarmMinute); // Nếu không có giá trị nào được lưu trước đó, sử dụng giá trị hiện tại của alarmMinute (mặc định là 0)
    prefs.end();
    Serial.printf("[ALARM] Loaded: %02d:%02d\n", alarmHour, alarmMinute);

    Serial.println("[SENSOR] RTC initialized");
}

// Hàm trả về con trỏ tới đối tượng DHT
// Dùng để truy cập cảm biến DHT từ các file khác
DHT *getDHT()
{
    return &dht;
}

// Hàm trả về con trỏ tới đối tượng RTC
// Dùng để truy cập module RTC từ các file khác
RTC_DS3231 *getRTC()
{
    return &rtc;
}