#include "log.hpp"

#define LOG_FILE_FORMAT "/sdcard/%d_log.txt"
#define MAX_FILE_BYTE_SIZE (1024 * 1000)

extern std::string string_vformat(const char* format, ...);

static constexpr const char logIds[4] = {
    'D',
    'I',
    'W',
    'E'
};

bool currentFile = false;
void CLog::m_RotateLogFile()
{
    // TODO: properly make new log files
    m_Log = CFile(string_vformat(LOG_FILE_FORMAT, (currentFile ^= 1)));
    m_Log.mWrite("");
}

CLog::CLog() : m_Log(string_vformat(LOG_FILE_FORMAT, currentFile)) {}

CLog& CLog::getInstance()
{
    static CLog instance = {};
    return instance;
}


void CLog::mInit() {}

void CLog::mWriteLog(const char* scope, const std::string& aText, LogType aType)
{
    time_t rawtime;
    time (&rawtime);
    struct tm timeinfo = *localtime(&rawtime);

    std::string time = string_vformat("[ %02d/%02d/%4d | %02d:%02d:%02d ]", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    // print to serial
    std::printf("%c %s (%s) %s\n", logIds[uint8_t(aType)], time.c_str(), scope, aText.c_str());
    
    // print to logfile
    try {
        m_Log.mAppend(string_vformat("%c ", logIds[uint8_t(aType)]));
        m_Log.mAppend(time);
        m_Log.mAppend(string_vformat(" (%s) ", scope));
        m_Log.mAppend(aText.c_str());
        m_Log.mAppend("\n");

        // File size check, if too big rotate the log file
        if (m_Log.mGetFileLength() > MAX_FILE_BYTE_SIZE) m_RotateLogFile();
    }
    catch (const std::runtime_error& e) {
        // Removed because clutter on serial log
        //std::printf("E %s (CLog.mWriteLog) Failed to write log to file: %s\n", time.c_str(), e.what());
    }
}

CLogScope CLog::mScope(const char* aScope)
{
    return CLogScope(aScope);
}
