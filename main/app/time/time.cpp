#include <time.hpp>
#include <wifi.hpp>
#include <string.h>
#include "esp_sntp.h"
#include <stdexcept>
#include <logscope.hpp>

static CLogScope logger{"time"};
#define TIMEZONE "CET-1CEST,M3.4.0/2,M10.4.0/2" //timezone NL

bool CTime::mb_IsInitialized = false;

void CTime::mInitTime(const char* apNtpServer) {
    auto& wifi = CWifi::getInstance();

    if (!wifi.mConnected()) throw std::runtime_error("Need a active internet connection to get the time.");

    logger.mDebug("Initializing SNTP");
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
