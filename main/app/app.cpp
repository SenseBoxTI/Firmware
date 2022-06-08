#include "app.hpp"
#include <stdexcept>
#include <iostream>
#include <sensormanager.hpp>
#include <dbsensor.hpp>
#include <o2sensor.hpp>
#include <scdsensor.hpp>
#include <colorsensor.hpp>
#include <lightintensitysensor.hpp>
#include <wifi.hpp>
#include <log.hpp>
#include <file.hpp>
#include <time.hpp>
#include <mqtt.hpp>
#include <config.hpp>

static CLogScope logger{"app"};

void App::init() {
    // Initialize Logger
    auto& log = CLog::getInstance();
    log.mInit();
    logger.mDebug("Application is starting");

    logger.mInfo("Initializing SD");
    // Initialize SD
    try {
        CFile::mInitSd();
    }
    catch (const std::runtime_error& e) {
        logger.mError("Initializing SD threw error: %s", e.what());
    }

    auto& config = CConfig::getInstance();
    config.mRead("/sdcard/config.toml");
    
    logger.mInfo("Initializing Wifi");
    // init PEAP network
    try {
        auto& wifiConfig = config["wifi"];

        WifiCredentials credentials = {
            .ssid = wifiConfig.get<std::string>("ssid"),
            .eapId = wifiConfig.get<std::string>("eapId"),
            .eapUsername = wifiConfig.get<std::string>("eapUsername"),
            .password = wifiConfig.get<std::string>("password")
        };

        CWifi::getInstance().mInitWifi(credentials);
    }
    catch (const std::runtime_error &e) {
        logger.mError("Error thrown while initing wifi: %s", e.what());
    }

    logger.mInfo("Initializing Time");
    try {
        CTime::mInitTime("pool.ntp.org");
    }
    catch (const std::runtime_error& e) {
        logger.mError("Initializing NTP threw error: %s", e.what());
    }

    auto& mqttConfig = config["mqtt"];

    std::string deviceId = mqttConfig.get<std::string>("deviceId");
    std::string accessToken = mqttConfig.get<std::string>("accessToken");

    auto& mqtt = CMqtt::getInstance();
    mqtt.mInit(deviceId, accessToken);

    auto& sensorManager = CSensorManager::getInstance();

    logger.mDebug("Adding sensors...");
    sensorManager.mAddSensor(new CDbSensor("dbSensor"));
    sensorManager.mAddSensor(new CO2Sensor("O2Sensor"));
    sensorManager.mAddSensor(new CScdSensor("SCDSensor"));
    sensorManager.mAddSensor(new CColorSpectrumSensor("ColorSpectrumSensor"));
    sensorManager.mAddSensor(new CLightIntensitySensor("LightIntensitySensor"));

    logger.mInfo("System has started.");
    logger.mInfo("The current time is: %s", CTime::mGetTimeString().c_str());
}

void App::loop() {
    auto& sensorManager = CSensorManager::getInstance();
    auto measurements = sensorManager.mMeasure();
    auto& mqtt = CMqtt::getInstance();

    // Print all measurements
    for (auto const &measurement: measurements) {
        logger.mInfo("Sensor '%s':", measurement.first.c_str());
        for (auto const &measurement : measurement.second) {
            logger.mInfo("\t%s\t: %s",
                measurement.first.c_str(),
                measurement.second.c_str()
            );
        }
        logger.mInfo("");
    }

    mqtt.mSendMeasurements(measurements);

    vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);
}

void App::start() {
    try {
        this->init();
        while (true) this->loop();
    }
    catch (const std::runtime_error& e) {
            logger.mError("Ah shucks!");
            logger.mError("FATAL unhandled runtime exception occured!");
            logger.mError("Famous lasts words:");
            logger.mError(e.what());
    }
}
