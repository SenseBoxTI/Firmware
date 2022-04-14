#pragma once

#include <measurements.hpp>
#include <logscope.hpp>
#include <logtype.hpp>
#include <file.hpp>

class CLog {
        CFile m_Log;
        void m_RotateLogFile();
        CLog();
        CLog(CLog&& other) = delete;
    public:
        static CLog& getInstance();
        void mInit();
        void mWriteLog(const char* scope, const std::string& aText, LogType aType);
        CLogScope mScope(const char* aScope);
};