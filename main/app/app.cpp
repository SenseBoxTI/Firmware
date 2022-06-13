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
#include <config.hpp>
#include <CConfig.hpp>

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
    sensorManager.mAddSensor(new CColorSpectrumSensor("ColorSpectrumSensor"));
    sensorManager.mAddSensor(new CLightIntensitySensor("LightIntensitySensor"));
    auto scdSensor = new CScdSensor("SCDSensor");
    sensorManager.mAddSensor(scdSensor);
    sensorManager.mAddSensor(new CParticleSensor("Particlesensor"));
    sensorManager.mAddSensor(new CVocSensor("VOCSensor", *scdSensor));

    logger.mInfo("System has started.");
    logger.mInfo("The current time is: %s", CTime::mGetTimeString().c_str());
}

void App::m_SendMeasurements(void* aArgs) {
    auto& sensorManager = CSensorManager::getInstance();
    auto measurements = sensorManager.mGetResults();
    auto& mqtt = CMqtt::getInstance();

    mqtt.mSendMeasurements(measurements);
}

void App::start() {
    try {
        this->init();

        esp_timer_create_args_t createArgs = {
            .callback = &m_SendMeasurements,
            .arg = this,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "SendData"
        };

        ESP_ERROR_CHECK(esp_timer_create(&createArgs, &m_SendDataTimer));

        ESP_ERROR_CHECK(esp_timer_start_periodic(m_SendDataTimer, SEND_INTERVAL_US));

        auto& wifi = CWifi::getInstance();
        auto& mqtt = CMqtt::getInstance();

        CWifi::WifiCb mqttConnectCb = [&]{mqtt.mReconnect();};
        CWifi::WifiCb mqttDisconnectCb = [&]{mqtt.mDisconnect();};

        wifi.mAttachOnConnect(mqttConnectCb);
        wifi.mAttachOnDisconnect(mqttDisconnectCb);
    }
    catch (const std::runtime_error& e) {
        logger.mError("Ah shucks!");
        logger.mError("FATAL unhandled runtime exception occured!");
        logger.mError("Famous lasts words:");
        logger.mError(e.what());

        auto& sensorManager = CSensorManager::getInstance();
        sensorManager.mClearTasks();

        auto& mqtt = CMqtt::getInstance();
        mqtt.mDisconnect();

        auto& wifi = CWifi::getInstance();
        wifi.mDisconnect();
    }
}
