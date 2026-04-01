#include "wifi_clock.h"
#include "ntp_sync.h"

void initWiFiAndNTP(const char *ssid, const char *pass, RTC_DS3231 *rtc)
{
    if (!ssid || ssid[0] == '\0')
    {
        Serial.println("[WiFi] No SSID configured - skip");
        return;
    }

    WiFi.mode(WIFI_STA);
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
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        return;
    }

    Serial.printf("\n[WiFi] Connected: %s\n",
                  WiFi.localIP().toString().c_str());

    // Sync NTP → RTC
    NTPSyncResult r = syncRTCfromNTP(rtc, 3, 8000);
    Serial.printf("[NTP] Result: %s\n", ntpResultStr(r));

    // Tắt WiFi sau sync — smart clock không cần WiFi thường trực
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("[WiFi] OFF - RTC holds time");
}