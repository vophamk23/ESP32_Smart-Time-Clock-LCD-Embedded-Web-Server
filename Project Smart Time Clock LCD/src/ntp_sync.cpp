#include "ntp_sync.h"
#include <WiFi.h>
#include <time.h>

static const char *NTP_SERVER_1 = "pool.ntp.org";
static const char *NTP_SERVER_2 = "time.google.com";
static const char *NTP_SERVER_3 = "time.cloudflare.com";
static const long GMT_OFFSET_SEC = 7 * 3600; // UTC+7 Việt Nam
static const int DAYLIGHT_OFFSET = 0;

static bool waitForNTP(uint32_t timeoutMs)
{
    struct tm timeinfo;
    uint32_t start = millis();
    while (!getLocalTime(&timeinfo, 1000))
    {
        if (millis() - start >= timeoutMs)
            return false;
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    return true;
}

NTPSyncResult syncRTCfromNTP(RTC_DS3231 *rtc,
                             uint8_t maxRetries,
                             uint32_t timeoutMs)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[NTP] No WiFi - skip sync");
        return NTPSyncResult::NO_WIFI;
    }

    if (!rtc)
    {
        Serial.println("[NTP] RTC pointer null");
        return NTPSyncResult::RTC_ERROR;
    }

    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET,
               NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

    struct tm timeinfo;
    bool synced = false;

    for (uint8_t i = 1; i <= maxRetries; i++)
    {
        Serial.printf("[NTP] Attempt %d/%d", i, maxRetries);
        if (waitForNTP(timeoutMs))
        {
            getLocalTime(&timeinfo);
            synced = true;
            break;
        }
        Serial.printf("[NTP] Attempt %d timed out\n", i);
        if (i < maxRetries)
            delay(2000);
    }

    if (!synced)
    {
        Serial.println("[NTP] All attempts failed");
        return NTPSyncResult::TIMEOUT;
    }

    // Ghi vào DS3231 qua pointer
    rtc->adjust(DateTime(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec));

    Serial.printf("[NTP] Synced: %02d/%02d/%04d %02d:%02d:%02d (UTC+7)\n",
                  timeinfo.tm_mday, timeinfo.tm_mon + 1,
                  timeinfo.tm_year + 1900,
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    return NTPSyncResult::SUCCESS;
}

const char *ntpResultStr(NTPSyncResult result)
{
    switch (result)
    {
    case NTPSyncResult::SUCCESS:
        return "SUCCESS";
    case NTPSyncResult::NO_WIFI:
        return "NO_WIFI";
    case NTPSyncResult::TIMEOUT:
        return "TIMEOUT";
    case NTPSyncResult::RTC_ERROR:
        return "RTC_ERROR";
    default:
        return "UNKNOWN";
    }
}