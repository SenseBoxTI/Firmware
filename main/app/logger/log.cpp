#include "log.hpp"
#include <dir.hpp>
#include <time.hpp>
#include <CConfig.hpp>

extern std::string string_vformat(const char* apFormat, ...);

static bool dirCheckDone = false;

static constexpr const char logIds[4] = {'D', 'I', 'W', 'E'};

static const char* resetStyle = "\x1b[0m";

static const char* ansiColors[4] = {
    "\x1b[38;5;120m", // debug
    "\x1b[38;5;250m", // info
    "\x1b[38;5;220m", // warning
    "\x1b[38;5;196m"  // Error
};

// TODO rotate log file using timestamps
void CLog::m_RotateLogFile(void* aSelf) {
    CLog& self = *static_cast<CLog*>(aSelf);
    self.m_RotateLogFile();
}

void CLog::m_RotateLogFile() {
    std::string timeString = CTime::mGetFormattedTimeString();

    try {
        printf("%sD %s (log) Checking log file size.%s\r\n", ansiColors[0], timeString.c_str(), resetStyle);

        if (m_Log.mGetFileLength() < LOG_MAX_FILE_SIZE_BYTE) return;

        printf("%sD %s (log) Rotating log file.%s\r\n", ansiColors[0], timeString.c_str(), resetStyle);

        m_Log.mRename(string_vformat("%u.log", CTime::mGetUnixTime()));
        m_Log = CDir(LOG_DIR).mFile("latest.log", Write);
        m_Log.mWrite("");
        m_Log.mReopen(Append);
    }
    catch (const std::runtime_error& e) {
        std::printf("%sE %s (log) Failed to rotate log file: %s%s\r\n", ansiColors[3], timeString.c_str(), e.what(), resetStyle);
        m_Log.mAppend(string_vformat("E %s (log) Failed to rotate log file: %s\r\n", timeString.c_str(), e.what()));
    }
}

CLog::CLog()
:   m_Log(LOG_DIR "/latest.log", Append),
    m_RotateTimer(nullptr)
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

void CLog::mInit() {
    m_RotateTimer = CTimer::mInit("LogRotate", &m_RotateLogFile, this);
    m_RotateTimer->mStartPeriodic(LOG_ROTATE_INTERVAL);
    m_Log.mStartWriteTimer();
}

void CLog::mWriteLog(const char* apScope, const std::string& arText, LogType aType) {
    // Get time string
    std::string timeString = CTime::mGetFormattedTimeString();

    std::string toPrint = string_vformat("%c %s (%s) %s\r\n",
        logIds[uint8_t(aType)],
        timeString.c_str(),
        apScope,
        arText.c_str()
    );

    // print to serial
    std::printf("%s%s%s", ansiColors[uint8_t(aType)], toPrint.c_str(), resetStyle);

    // print to logfile
    if (CFile::getSdState() == SdState::Ready) {
        try {
            if (!dirCheckDone) {
                CDir(LOG_DIR).mEnsure();
                dirCheckDone = true;
            }

            m_Log.mAppend(toPrint);
        }
        catch (const std::runtime_error& e) {
            std::printf("%sE %s (log) Failed to write log to file: %s%s\r\n", ansiColors[3], timeString.c_str(), e.what(), resetStyle);
        }
    }
}

void CLog::mFinalize() {
    m_Log.mFlushQueue();
}

CLogScope CLog::mScope(const char* apScope) {
    return CLogScope(apScope);
}
