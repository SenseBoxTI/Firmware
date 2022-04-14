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

    char timestr[26];
    memset(timestr, 0, sizeof(timestr));
    strftime(timestr, sizeof(timestr), "[ %d/%m/%Y | %H:%M:%S ]", &timeinfo);
    // print to serial
    std::printf("%c %s (%s) %s\n", logIds[uint8_t(aType)], timestr, scope, aText.c_str());
    
    // print to logfile
    try {
        m_Log.mAppend(string_vformat("%c ", logIds[uint8_t(aType)]));
        m_Log.mAppend(std::string(timestr, sizeof(timestr) - 1));
        m_Log.mAppend(string_vformat(" (%s) ", scope));
        m_Log.mAppend(aText.c_str());
        m_Log.mAppend("\n");

        // File size check, if too big rotate the log file
        if (m_Log.mGetFileLength() > MAX_FILE_BYTE_SIZE) m_RotateLogFile();
    }
    catch (const std::runtime_error& e) {
        // Removed because clutter on serial log
        //std::printf("E %s (CLog.mWriteLog) Failed to write log to file: %s\n", timestr, e.what());
    }
}

CLogScope CLog::mScope(const char* aScope)
{
    return CLogScope(aScope);
}
