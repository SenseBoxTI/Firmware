#include "log.hpp"

#define LOG_FILE_FORMAT "%d_log.txt"
#define MAX_FILE_BYTE_SIZE (1024 * 1000)

extern std::string string_vformat(const char* apFormat, ...);

static constexpr const char logIds[4] = {
    'D',
    'I',
    'W',
    'E'
};

static const char* resetStyle = "\x1b[0m";

static const char* ansiColors[4] = {
    "\x1b[38;5;120m",
    "\x1b[38;5;250m",
    "\x1b[38;5;220m",
    "\x1b[38;5;196m"
};

bool currentFile = false;
void CLog::m_RotateLogFile() {
    // TODO: properly make new log files
    m_Log = CFile(string_vformat(LOG_FILE_FORMAT, (currentFile ^= 1)));
    m_Log.mWrite("");
}

CLog::CLog() : m_Log(string_vformat(LOG_FILE_FORMAT, currentFile)) {}

CLog& CLog::getInstance() {
    static CLog instance = {};
    return instance;
}

void CLog::mInit() {}

void CLog::mWriteLog(const char* apScope, const std::string& arText, LogType aType) {
    // TODO: eventually 
    time_t rawtime;
    time (&rawtime);
    struct tm timeinfo = *localtime(&rawtime);

    std::string time = string_vformat("[ %02d/%02d/%4d | %02d:%02d:%02d ]",
        timeinfo.tm_mday,
        timeinfo.tm_mon + 1, // tm mon 0-11
        timeinfo.tm_year + 1900, // tm years since 1900
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
    );
    
    std::string toPrint = string_vformat("%c %s (%s) %s",
        logIds[uint8_t(aType)],
        time.c_str(),
        apScope,
        arText.c_str()
    );

    // print to serial
    std::printf("%s%s%s\n", ansiColors[uint8_t(aType)], toPrint.c_str(), resetStyle);

    // print to logfile
    if (CFile::getSdState() == SdState::Ready) {
        try {
            m_Log.mAppend(toPrint);
            m_Log.mAppend("\r\n");
            // File size check, if too big rotate the log file
            if (m_Log.mGetFileLength() > MAX_FILE_BYTE_SIZE) m_RotateLogFile();
        }
        catch (const std::runtime_error& e) {
            std::printf("%sE %s (CLog.mWriteLog) Failed to write log to file: %s%s\r\n", ansiColors[3], time.c_str(), e.what(), resetStyle);
        }
    }
}

CLogScope CLog::mScope(const char* apScope) {
    return CLogScope(apScope);
}
