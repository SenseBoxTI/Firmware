#pragma once

#include <CTimer.hpp>
#include <config.hpp>

typedef enum {
    Init = 0,
    Active,
    SoftReset,
    Stopped
} AppState;

class App {
public:
    static App& getInstance();
    void start();
    static void exit(const std::exception& e);

    static AppState status;

private:
    App();
    void init();
    static void m_SendMeasurements(void* aArgs);

    void initSdCard();
    void initWifi(toml::Value config);
    void initNtp();
    void initMqtt(toml::Value config);
    void initSensors();
    void startSendingData();
    void attachWifiCallbacks();
    uint8_t deinit();

    CTimer* m_SendDataTimer;
};