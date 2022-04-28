#pragma once
#include <string.h>

class CLogScope {
private:
    const char* m_Scope;
public:
    CLogScope(const char* apScope);
    void mInfo(const char* apFormat, ...);
    void mWarn(const char* apFormat, ...);
    void mError(const char* apFormat, ...);
    void mDebug(const char* apFormat, ...);
};
