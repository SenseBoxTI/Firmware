#pragma once

#include <measurements.hpp>
#include <logscope.hpp>
#include <logtype.hpp>
#include <file.hpp>

#include <CTimer.hpp>

class CLog {
private:
    CFile m_Log;
    static void m_RotateLogFile(void* aSelf);
    void m_RotateLogFile();
    CLog();
    CLog(CLog&& rrOther) = delete;

public:
    static CLog& getInstance();
    void mInit();
    void mWriteLog(const char* apScope, const std::string& arText, LogType aType);
    CLogScope mScope(const char* apScope);

    CTimer* m_RotateTimer;
};
