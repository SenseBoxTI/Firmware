#pragma once
#include <time.h>
#include <string>

class CTime {
public:
    static void mInitTime(const char* apNtpServer);
    static struct tm mGetTime();
    static time_t mGetUnixTime();
    static std::string mGetTimeString();
    static std::string mGetFormattedTimeString();

    CTime() = delete;

private:
    static bool mb_IsInitialized;
};
