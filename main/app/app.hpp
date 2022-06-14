#pragma once

#include <esp_timer.h>
#include <config.hpp>

class App {
    void init();
    static void m_SendMeasurements(void* aArgs);

    void initSdCard();
    void initWifi(toml::Value config);
    void initNtp();
    void initMqtt(toml::Value config);
    void initSensors();
    void startSendingData();
    void attachWifiCallbacks();

    esp_timer_handle_t m_SendDataTimer;

public:
    void start();
};