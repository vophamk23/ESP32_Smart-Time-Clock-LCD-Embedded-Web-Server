#ifndef NTP_SYNC_H
#define NTP_SYNC_H

#include <Arduino.h>
#include <RTClib.h>

enum class NTPSyncResult
{
    SUCCESS,
    NO_WIFI,
    TIMEOUT,
    RTC_ERROR
};

// rtc: truyền vào từ getRTC()
NTPSyncResult syncRTCfromNTP(RTC_DS3231 *rtc,
                             uint8_t maxRetries = 3,
                             uint32_t timeoutMs = 5000);

const char *ntpResultStr(NTPSyncResult result);

#endif // NTP_SYNC_H