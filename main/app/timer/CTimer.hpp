#pragma once

#include <esp_timer.h>

class CTimer {
public:
    CTimer() = delete;

    static CTimer* mInit(const char* apName, esp_timer_cb_t aCb, void* aUserCtx);
    void mStop();
    void mStartPeriodic(uint64_t aInterval);
    void mStartOnce(uint64_t aInterval);
    void mDelete();
    void m_RunCallback();

    const char* mName;

private:
    CTimer(const char* apName, esp_timer_cb_t aCb, void* aUserCtx, esp_timer_handle_t** aTimerHandle);
    ~CTimer();

    typedef enum {
        NO_CHECK,
        CHECK,
        LOG
    } ActiveCheck;

    void m_StopIfActive(ActiveCheck checkIfRunning = CHECK);

    esp_timer_cb_t m_Cb;
    void* m_UserCtx;
    uint64_t m_Interval;

    esp_timer_handle_t m_Timer;

    friend class CTimers;
};
