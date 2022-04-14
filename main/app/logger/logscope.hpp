#pragma once
#include <string.h>

class CLogScope {
    const char* m_Scope;
    friend class CLog;
        CLogScope(const char* aScope);
    public:
        void mInfo(const char* aFormat, ...);
        void mWarn(const char* aFormat, ...);
        void mError(const char* aFormat, ...);
        void mDebug(const char* aFormat, ...);
        void mThrow(const char* aFormat, ...);
};