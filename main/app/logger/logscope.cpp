#include "logscope.hpp"
#include "log.hpp"
#include <string>
#include <stdarg.h>

#include <CConfig.hpp>

std::string string_vformat(const char* apFormat, va_list aArgs) {
    // get size
    size_t size = vsnprintf(nullptr, 0, apFormat, aArgs) + 1;
    // error
    if (size <= 0) return "";

    // put scope into result
    std::string result;
    result.resize(size);
    // print rest after scope
    vsnprintf(&result[0], size, apFormat, aArgs);
    return result;
}

std::string string_vformat(const char* apFormat, ...) {
    va_list a_list;
    va_start(a_list, apFormat);
    std::string result = string_vformat(apFormat, a_list);
    va_end(a_list);
    return result;
}

CLogScope::CLogScope(const char* apScope)
:   m_Scope(apScope)
{}

void CLogScope::mInfo(const char* apFormat, ...) {
    va_list a_list;
    va_start(a_list, apFormat);
    CLog::getInstance().mWriteLog(m_Scope, string_vformat(apFormat, a_list), ::LogType::Info);
    va_end(a_list);
}

void CLogScope::mWarn(const char* apFormat, ...) {
    va_list a_list;
    va_start(a_list, apFormat);
    CLog::getInstance().mWriteLog(m_Scope, string_vformat(apFormat, a_list), ::LogType::Warning);
    va_end(a_list);
}

void CLogScope::mError(const char* apFormat, ...) {
    va_list a_list;
    va_start(a_list, apFormat);
    CLog::getInstance().mWriteLog(m_Scope, string_vformat(apFormat, a_list), ::LogType::Error);
    va_end(a_list);
}

void CLogScope::mDebug(const char* apFormat, ...) {
    if (!DEBUG_ENABLED) return;

    va_list a_list;
    va_start(a_list, apFormat);
    CLog::getInstance().mWriteLog(m_Scope, string_vformat(apFormat, a_list), ::LogType::Debug);
    va_end(a_list);
}
