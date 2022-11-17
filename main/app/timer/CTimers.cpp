#include <CTimers.hpp>

#include <string>
#include <logscope.hpp>
#include <app.hpp>

static CLogScope logger{"timers"};

CTimers::CTimers() {}

CTimers::~CTimers() {
    mCleanTimers();
}

CTimers& CTimers::getInstance() {
    static auto instance = CTimers();
    return instance;
}

CTimer* CTimers::mAddTimer(const char* apName, esp_timer_cb_t aCb, void* aUserCtx) {
    if (m_Timers.find(apName) != m_Timers.end()) throw std::runtime_error("Timer '" + std::string(apName) + "' was already created.");

    esp_timer_handle_t* timerHandler;

    CTimer* timer = new CTimer(apName, aCb, aUserCtx, &timerHandler);

    auto inserted = m_Timers.emplace(apName, timer);
    if (inserted.second != true) throw std::runtime_error("Could not insert timer '" + std::string(apName) + "'.");

    esp_timer_create_args_t createArgs = {
        .callback = &m_RunCallback,
        .arg = timer,
        .dispatch_method = ESP_TIMER_TASK,
        .name = apName
    };

    esp_timer_create(&createArgs, timerHandler);

    return timer;
}

void CTimers::mRemoveTimer(const char* apName) {
    auto res = m_Timers.find(apName);

    if (res == m_Timers.end()) {
        logger.mWarn("Timer '%s' does not exist.", apName);
        return;
    }

    // delete the pointer, then remove the item from the queue
    delete res->second;
    m_Timers.erase(apName);
}

/**
 * stops all tasks
 * only run after the program has reached a fatal error
 */
void CTimers::mCleanTimers() {
    logger.mInfo("Starting cleanup of timers.");

    // delete each item in the queue
    for (auto& tp : m_Timers) {
        auto timer = tp.second;

        delete timer;
    }

    // empty the queue
    m_Timers.clear();

    logger.mInfo("All timers were stopped.");
}

void CTimers::m_RunCallback(void* apArg) {
    CTimer& ctx = *static_cast<CTimer*>(apArg);

    try {
        ctx.m_RunCallback();
    }
    catch (const std::exception& e) {
        logger.mError("Task error:");
        App::softRestart(e);
    }
}

bool CTimers::mCheckTimerExists(const char* apName) {
    auto& self = CTimers::getInstance();

    if (self.m_Timers.find(apName) == self.m_Timers.end()) return false;
    return true;
}
