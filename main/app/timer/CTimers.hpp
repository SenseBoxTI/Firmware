#pragma once

#include <map>
#include <CTimer.hpp>

class CTimers {
public:
    /// @brief get instance of CTimer
    static CTimers& getInstance();

    CTimer* mAddTimer(const char* apName, esp_timer_cb_t aCb, void* aUserCtx);
    void mRemoveTimer(const char* apName);
    void mCleanTimers();
    static bool mCheckTimerExists(const char* apName);

private:
    CTimers();
    ~CTimers();

    static void m_RunCallback(void* apArg);

    std::map<const char*, CTimer*> m_Timers;
};
