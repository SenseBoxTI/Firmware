#pragma once

#include <measurements.hpp>
#include <logscope.hpp>
#include <logtype.hpp>
#include <file.hpp>

class CLog {
        CFile m_Log;
        void m_RotateLogFile();
        CLog();
        CLog(CLog&& rrOther) = delete;
    public:
        static CLog& getInstance();
        void mInit();
        void mWriteLog(const char* apScope, const std::string& arText, LogType aType);
        CLogScope mScope(const char* apScope);
};
