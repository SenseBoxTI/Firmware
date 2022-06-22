#include <time.hpp>
#include <wifi.hpp>
#include <string.h>
#include "esp_sntp.h"
#include <stdexcept>
#include <logscope.hpp>
#include <CConfig.hpp>

static CLogScope logger{"time"};

bool CTime::mb_IsInitialized = false;

extern std::string string_vformat(const char* apFormat, ...);

void CTime::mInitTime(const char* apNtpServer) {
    auto& wifi = CWifi::getInstance();

    if (!wifi.mConnected()) throw std::runtime_error("Need a active internet connection to get the time.");

    logger.mDebug("Initializing NTP");
    sntp_servermode_dhcp(1);
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(1, apNtpServer);
    sntp_setservername(0, apNtpServer);
    sntp_init();

    uint8_t retry = 0;
    const uint8_t retryCnt = 10;

    logger.mInfo("Waiting for system time to be set...");
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry <= retryCnt) {
        logger.mDebug("Waiting...  (%d/%d)", retry, retryCnt);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    // check if we have time
    if (retry == retryCnt) throw std::runtime_error("Could not get the time from NTP server.");

    setenv("TZ", TIMEZONE, 1);
    tzset();

    mb_IsInitialized = true;
}

struct tm CTime::mGetTime() {
    time_t rawtime = mGetUnixTime(); // unix timestamp value

    return *localtime(&rawtime);
}

time_t CTime::mGetUnixTime() {
    if (!mb_IsInitialized) throw std::runtime_error("Time was not initialized first.");

    time_t rawtime; // unix timestamp value
    time(&rawtime);

    return rawtime;
}

std::string CTime::mGetTimeString() {
    char strftime_buf[32];

    struct tm timeinfo = mGetTime();

    size_t l = strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    return std::string(strftime_buf, l);
}

std::string CTime::mGetFormattedTimeString() {
    std::string timeString;

    try {
        struct tm timeinfo = CTime::mGetTime();
        timeString = string_vformat(
            "[ %02d/%02d/%4d | %02d:%02d:%02d ]",
            timeinfo.tm_mday,
            timeinfo.tm_mon + 1, // tm mon 0-11
            timeinfo.tm_year + 1900, // tm years since 1900
            timeinfo.tm_hour,
            timeinfo.tm_min,
            timeinfo.tm_sec
        );
    }
    catch (const std::runtime_error& e) {
        timeString = "[  TIME  UNINITIALIZED  ]";
    }

    return timeString;
}
