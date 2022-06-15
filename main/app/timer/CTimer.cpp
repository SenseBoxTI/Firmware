#include <CTimer.hpp>

#include <string>
#include <logscope.hpp>
#include <CTimers.hpp>

static CLogScope logger{"timer"};

static auto& timers = CTimers::getInstance();

CTimer::CTimer(const char* apName, esp_timer_cb_t aCb, void* aUserCtx, esp_timer_handle_t** aTimerHandle)
:   mName(apName),
    m_Cb(aCb),
    m_UserCtx(aUserCtx),
    m_Interval(0)
{
    *aTimerHandle = &m_Timer;
}

CTimer::~CTimer() {
    if (m_Timer == NULL) return;

    m_StopIfActive(NO_CHECK);
    esp_timer_delete(m_Timer);
}

CTimer* CTimer::mInit(const char* apName, esp_timer_cb_t aCb, void* aUserCtx) {
    return timers.mAddTimer(apName, aCb, aUserCtx);
}

void CTimer::mStop() {
    m_StopIfActive(LOG);
}

void CTimer::mStartPeriodic(uint64_t aInterval) {
    m_StopIfActive();

    m_Interval = aInterval;

    esp_timer_start_periodic(m_Timer, m_Interval);
}

void CTimer::mStartOnce(uint64_t aInterval) {
    m_StopIfActive();

    m_Interval = aInterval;

    esp_timer_start_once(m_Timer, m_Interval);
}

void CTimer::mDelete() {
    timers.mRemoveTimer(mName);
}

void CTimer::m_StopIfActive(ActiveCheck check) {
    if (check == CHECK && !esp_timer_is_active(m_Timer)) return;
    if (esp_timer_stop(m_Timer) == ESP_ERR_INVALID_STATE && check == LOG) {
        logger.mDebug("Timer %s was not running", mName);
    }
}

void CTimer::m_RunCallback() {
    m_Cb(m_UserCtx);
}
