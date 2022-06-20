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

/**
 * The application has a logger which can be used to initialize different `scopes'.
 * You can imagine these as printing wrappers.
 * They also print to the log file, unlike `printf'
 */
static CLogScope logger{"app"};

bool App::stopped = false;
bool App::handledException = false;

App::App()
:   m_SendDataTimer(nullptr)
{}

/**
 * Measurements are taken from the Sensor manager and send over MQTT.
 */
void App::m_SendMeasurements(void* aArgs) {
    auto& sensorManager = CSensorManager::getInstance();
    auto measurements = sensorManager.mGetResults();
    auto& mqtt = CMqtt::getInstance();

    mqtt.mSendMeasurements(measurements);
}

/**
 * Application entry point
 */
void App::start() {
    try {
        init();
    }
    catch (const std::runtime_error& e) {
        exit(e);
    }
}

/**
 * Initialize all components
 * Function calls should be self explanatory
 */
void App::init() {
    logger.mDebug("Application is starting");

    initSdCard();

    auto& config = CConfig::getInstance();
    config.mRead("/sdcard/config.toml");

    CLog::getInstance().mInit();

    initWifi(config["wifi"]);
    initNtp();
    initMqtt(config["mqtt"]);
    initSensors();
    attachWifiCallbacks();
    startSendingData();

    logger.mInfo("System has started.");
    logger.mInfo("The current time is: %s", CTime::mGetTimeString().c_str());
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

void App::initMqtt(toml::Value config) {
    try {
        std::string deviceId = config.get<std::string>("deviceId");
        std::string accessToken = config.get<std::string>("accessToken");

        auto& mqtt = CMqtt::getInstance();
        mqtt.mInit(deviceId, accessToken);
    }
    catch (const std::runtime_error& e) {
        logger.mError("Initializing MQTT threw error: %s", e.what());

        throw std::runtime_error(e); // or do some flashy things with the LED
    }
}

void App::initSensors() {
    auto& sensorManager = CSensorManager::getInstance();

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

    CWifi::WifiCb mqttConnectCb = [&]{mqtt.mReconnect();};
    CWifi::WifiCb mqttDisconnectCb = [&]{mqtt.mDisconnect();};

    wifi.mAttachOnConnect(mqttConnectCb);
    wifi.mAttachOnDisconnect(mqttDisconnectCb);

    logger.mDebug("Attached WiFi connection callbacks");
}

/**
 * Create a task that sends data periodically every `SEND_INTERVAL_US` as defined in the global configuration
 */
void App::startSendingData() {
    m_SendDataTimer = CTimer::mInit("SendData", &m_SendMeasurements, this);
    m_SendDataTimer->mStartPeriodic(SEND_INTERVAL_US);

    logger.mDebug("Send data timer has started");
}

/**
 * Exit and deinitialize as much as possible.
 * This is usually never called, so something must've gone critically wrong.
 */
void App::exit(const std::exception& e) {
    logger.mError("Ah shucks!");
    logger.mError("FATAL unhandled runtime exception occured!");
    logger.mError("Famous last words:");
    logger.mError(e.what());

    if (App::handledException) return;
    App::handledException = true;

    auto& mqtt = CMqtt::getInstance();
    auto& wifi = CWifi::getInstance();
    auto& log = CLog::getInstance();
    auto& timers = CTimers::getInstance();

    mqtt.mDisconnect();
    wifi.mDisconnect();
    timers.mCleanTimers();
    log.mFinalize();

    CFile::mDeinitSd();

    App::stopped = true;
}
