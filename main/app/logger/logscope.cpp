#include "logscope.hpp"
#include "log.hpp"
#include <string>
#include <stdarg.h>

std::string string_vformat(const char* format, va_list args) {
    // get size
    size_t size = vsnprintf(nullptr, 0, format, args) + 1;
    // error
    if (size <= 0)
        return "";
    
    // put scope into result
    std::string result;
    result.resize(size);
    // print rest after scope
    vsnprintf(&result[0], size, format, args);
    return result;
}

std::string string_vformat(const char* format, ...) {
    va_list a_list;
    va_start(a_list, format);
    std::string result = string_vformat(format, a_list);
    va_end(a_list);
    return result;
}

CLogScope::CLogScope(const char* aScope) : m_Scope(aScope) {}

void CLogScope::mInfo(const char* aFormat, ...) {
    va_list a_list;
    va_start(a_list, aFormat);
    CLog::getInstance().mWriteLog(m_Scope, string_vformat(aFormat, a_list), ::LogType::Info);
    va_end(a_list);
}

void CLogScope::mWarn(const char* aFormat, ...) {
    va_list a_list;
    va_start(a_list, aFormat);
    CLog::getInstance().mWriteLog(m_Scope, string_vformat(aFormat, a_list), ::LogType::Warning);
    va_end(a_list);
}

void CLogScope::mError(const char* aFormat, ...) {
    va_list a_list;
    va_start(a_list, aFormat);
    CLog::getInstance().mWriteLog(m_Scope, string_vformat(aFormat, a_list), ::LogType::Error);
    va_end(a_list);
}

void CLogScope::mDebug(const char* aFormat, ...) {
    va_list a_list;
    va_start(a_list, aFormat);
    CLog::getInstance().mWriteLog(m_Scope, string_vformat(aFormat, a_list), ::LogType::Debug);
    va_end(a_list);
}

void CLogScope::mThrow(const char* aFormat, ...) {
    va_list a_list;
    va_start(a_list, aFormat);
    std::string result = string_vformat(aFormat, a_list);
    va_end(a_list);
    CLog::getInstance().mWriteLog(m_Scope, result, ::LogType::Error);
    throw std::runtime_error(result);
}