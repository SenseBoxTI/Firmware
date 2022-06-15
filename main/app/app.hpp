#pragma once

#include <CTimer.hpp>
#include <config.hpp>

class App {
public:
    App();
    void start();
    static void exit(const std::exception& e);

    static bool stopped;

private:
    void init();
    static void m_SendMeasurements(void* aArgs);
    static bool handledException;

    void initSdCard();
    void initWifi(toml::Value config);
    void initNtp();
    void initMqtt(toml::Value config);
    void initSensors();
    void startSendingData();
    void attachWifiCallbacks();

    CTimer* m_SendDataTimer;
};