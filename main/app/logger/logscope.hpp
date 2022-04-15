#pragma once
#include <string.h>

class CLogScope {
    const char* m_Scope;
    friend class CLog;
        CLogScope(const char* apScope);
    public:
        void mInfo(const char* apFormat, ...);
        void mWarn(const char* apFormat, ...);
        void mError(const char* apFormat, ...);
        void mDebug(const char* apFormat, ...);
        void mThrow(const char* apFormat, ...);
};