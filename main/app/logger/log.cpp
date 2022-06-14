#include "log.hpp"
#include <dir.hpp>
#include <time.hpp>

#define LOG_DIR "logs"
#define MAX_FILE_BYTE_SIZE (1024 * 1024) // 1MB

extern std::string string_vformat(const char* apFormat, ...);

static constexpr const char logIds[4] = {'D', 'I', 'W', 'E'};

static const char* resetStyle = "\x1b[0m";

static const char* ansiColors[4] = {
    "\x1b[38;5;120m", // debug
    "\x1b[38;5;250m", // info
    "\x1b[38;5;220m", // warning
    "\x1b[38;5;196m"  // Error
};

// TODO rotate log file using timestamps
void CLog::m_RotateLogFile() {
    m_Log.mRename(string_vformat("%u.log", CTime::mGetUnixTime()));
    m_Log = CDir(LOG_DIR).mFile("latest.log");
    m_Log.mWrite("");
}

CLog::CLog()
:   m_Log(LOG_DIR "/latest.log", Append)
{}

CLog& CLog::getInstance() {
    try {
        static CLog instance = {};
        return instance;
    }
    catch(const std::runtime_error& e) {
        std::printf(e.what());
        throw e;
    }
}

void CLog::mInit() {}

void CLog::mWriteLog(const char* apScope, const std::string& arText, LogType aType) {
    // Get time string
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

    std::string toPrint = string_vformat("%c %s (%s) %s",
        logIds[uint8_t(aType)],
        timeString.c_str(),
        apScope,
        arText.c_str()
    );

    // print to serial
    std::printf("%s%s%s\n", ansiColors[uint8_t(aType)], toPrint.c_str(), resetStyle);

    // print to logfile
    if (CFile::getSdState() == SdState::Ready) {
        try {
            CDir(LOG_DIR).mEnsure();
            m_Log.mAppend(toPrint);
            m_Log.mAppend("\r\n");
            // File size check, if too big rotate the log file
            try {
                if (m_Log.mGetFileLength() > MAX_FILE_BYTE_SIZE) m_RotateLogFile();
            }
            catch (const std::runtime_error& e) {
                std::printf("%sE %s (log) Failed to rotate log file: %s%s\r\n", ansiColors[3], timeString.c_str(), e.what(), resetStyle);
                m_Log.mAppend(string_vformat("E %s (log) Failed to rotate log file: %s\r\n", timeString.c_str(), e.what()));

            }
        }
        catch (const std::runtime_error& e) {
            std::printf("%sE %s (log) Failed to write log to file: %s%s\r\n", ansiColors[3], timeString.c_str(), e.what(), resetStyle);
        }
    }
}

CLogScope CLog::mScope(const char* apScope) {
    return CLogScope(apScope);
}
