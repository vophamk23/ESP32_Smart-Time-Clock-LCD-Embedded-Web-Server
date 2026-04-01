#include "wifi_clock.h"
#include "ntp_sync.h"

// ==========================================================================
// KHỞI TẠO WIFI + NTP
// ==========================================================================
void initWiFiAndNTP(const char *ssid, const char *pass, RTC_DS3231 *rtc)
{
    if (!ssid || ssid[0] == '\0')
    {
        Serial.println("[WiFi] No SSID configured - skip");
        return;
    }

    // --- Kết nối WiFi ---
    WiFi.mode(WIFI_STA); // Chỉ bật WiFi STA, để initWebServer() tự bật AP nếu cần
    WiFi.begin(ssid, (pass && pass[0] != '\0') ? pass : nullptr);
    Serial.printf("[WiFi] Connecting to %s", ssid);

    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000)
    {
        delay(300);
        Serial.print(".");
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("\n[WiFi] Timeout - RTC keeps old time");
        // KHÔNG tắt WiFi, để initWebServer() tự bật AP
        return;
    }

    Serial.printf("\n[WiFi] Connected: %s\n", WiFi.localIP().toString().c_str());

    // --- Đồng bộ NTP → RTC ---
    NTPSyncResult r = syncRTCfromNTP(rtc, 3, 8000);
    Serial.printf("[NTP] Result: %s\n", ntpResultStr(r));

    // ← XÓA 2 dòng WiFi.disconnect + WiFi.mode(WIFI_OFF)
    Serial.println("[WiFi] Staying connected for web dashboard");
}