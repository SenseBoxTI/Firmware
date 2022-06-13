#pragma once

#include <esp_timer.h>

class App {
    void init();
    static void m_SendMeasurements(void* aArgs);

    esp_timer_handle_t m_SendDataTimer;

public:
    void start();
};