#include "app.hpp"
#include <stdexcept>
#include <iostream>
#include <sensormanager.hpp>
#include <dbsensor.hpp>
#include <o2sensor.hpp>
#include <scdsensor.hpp>
#include <colorsensor.hpp>
#include <lightintensitysensor.hpp>
#include <particlesensor.hpp>
#include <vocsensor.hpp>
#include <wifi.hpp>
#include <log.hpp>
#include <file.hpp>
#include <time.hpp>
#include <mqtt.hpp>
#include <CConfig.hpp>
#include <CTimers.hpp>

static CLogScope logger{"app"};

AppState App::status = Init;

App::App()
:   m_SendDataTimer(nullptr)
{}

App& App::getInstance() {
    static auto instance = App();
    return instance;
}

void App::m_SendMeasurements(void* aArgs) {
    auto& sensorManager = CSensorManager::getInstance();
    auto measurements = sensorManager.mGetResults();
    auto& mqtt = CMqtt::getInstance();

    mqtt.mSendMeasurements(measurements);
}

void App::start() {
    try {
        init();
    }
    catch (const std::runtime_error& e) {
        softRestart(e);
    }
}

void App::init() {
    try {
        logger.mDebug("Application is starting");

        CLog::getInstance().mInit();
        initSdCard();

        auto& config = CConfig::getInstance();
        config.mRead("/sdcard/config.toml");

        initWifi(config["wifi"]);
        if (status == Init) initNtp();
        initMqtt();
        initSensors();
        attachWifiCallbacks();
        startSendingData();

        status = Active;

        logger.mInfo("System has started.");
        logger.mInfo("The current time is: %s", CTime::mGetTimeString().c_str());
    } catch (const std::runtime_error& e) {
        logger.mError("Error during init:");
        softRestart(e);
    }
}

void App::initSdCard() {
    try {
        CFile::mInitSd();
    }
    catch (const std::runtime_error& e) {
        logger.mError("Initializing SD threw error: %s", e.what());

        throw std::runtime_error(e); // or do some flashy things with the LED
    }
}

void App::initWifi(toml::Value config) {
    try {
        WifiCredentials credentials = {
            .ssid = config.get<std::string>("ssid"),
            .eapId = config.get<std::string>("eapId"),
            .eapUsername = config.get<std::string>("eapUsername"),
            .password = config.get<std::string>("password")
        };

        CWifi::getInstance().mInitWifi(credentials);
    }
    catch (const std::runtime_error &e) {
        logger.mError("Initializing WiFi threw error: %s", e.what());

        throw std::runtime_error(e); // or do some flashy things with the LED
    }
}

void App::initNtp() {
    try {
        CTime::mInitTime("pool.ntp.org");
    }
    catch (const std::runtime_error& e) {
        logger.mError("Initializing NTP threw error: %s", e.what());

        throw std::runtime_error(e); // or do some flashy things with the LED
    }
}

void App::initMqtt() {
    try {
        auto& mqtt = CMqtt::getInstance();
        if (status == Init) mqtt.mInit();
        else mqtt.mStartClient();
    }
    catch (const std::runtime_error& e) {
        logger.mError("Initializing MQTT threw error: %s", e.what());

        throw std::runtime_error(e); // or do some flashy things with the LED
    }
}

void App::initSensors() {
    auto& sensorManager = CSensorManager::getInstance();

    if (status == SoftReset) {
        sensorManager.mReinit();
        return;
    }

    logger.mDebug("Adding sensors...");
    sensorManager.mAddSensor(new CDbSensor("dbSensor"));
    sensorManager.mAddSensor(new CO2Sensor("O2Sensor"));
    sensorManager.mAddSensor(new CColorSpectrumSensor("ColorSpectrumSensor"));
    sensorManager.mAddSensor(new CLightIntensitySensor("LightIntensitySensor"));
    auto scdSensor = new CScdSensor("SCDSensor");
    sensorManager.mAddSensor(scdSensor);
    sensorManager.mAddSensor(new CParticleSensor("Particlesensor"));
    sensorManager.mAddSensor(new CVocSensor("VOCSensor", *scdSensor));
}

void App::attachWifiCallbacks() {
    auto& wifi = CWifi::getInstance();
    auto& mqtt = CMqtt::getInstance();

    CWifi::WifiCb mqttConnectCb = [&]{mqtt.mStartClient();};
    CWifi::WifiCb mqttDisconnectCb = [&]{mqtt.mDeinit();};

    wifi.mAttachOnConnect(mqttConnectCb);
    wifi.mAttachOnDisconnect(mqttDisconnectCb);

    logger.mDebug("Attached WiFi connection callbacks");
}

void App::startSendingData() {
    m_SendDataTimer = CTimer::mInit("SendData", &m_SendMeasurements, this);
    m_SendDataTimer->mStartPeriodic(SEND_INTERVAL_US);

    logger.mDebug("Send data timer has started");
}

uint8_t App::deinit() {
    auto& mqtt = CMqtt::getInstance();
    auto& wifi = CWifi::getInstance();
    auto& log = CLog::getInstance();
    auto& timers = CTimers::getInstance();

    uint8_t step = 0;

    try {
        step++;
        mqtt.mDeinit();

        step++;
        wifi.mDeinit();

        step++;
        timers.mCleanTimers();

        step++;
        log.mFinalize();

        step++;
        CFile::mDeinitSd();

        return 0;
    }
    catch (const std::exception& e) {
        logger.mError("Error during deinit: %s", e.what());
        logger.mError("Failed at step: %d", step);
        return step;
    }
}

void App::softRestart(const std::exception& e) {
    logger.mError("Ah shucks!");
    logger.mError("FATAL unhandled runtime exception occured!");
    logger.mError("Famous last words:");
    logger.mError(e.what());

    if (status == SoftReset) {
        logger.mWarn("Above error was received during soft reset status.");
        return;
    }
    status = SoftReset;

    auto& app = App::getInstance();

    if (app.deinit()) {
        status = Stopped;
        return;
    }

    try {
        logger.mWarn("App will restart now!");
        app.init();
    } catch (const std::exception& e) {
        logger.mError("Error during softrestart: %s", e.what());
        status = Stopped;
    }
}
