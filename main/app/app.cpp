#include "app.hpp"
#include <stdexcept>
#include <iostream>
#include <sensormanager.hpp>
#include <dbsensor.hpp>
#include <wifi.hpp>
#include <log.hpp>
#include <file.hpp>
#include <sd.hpp>


void App::init() {
    // Initialize SD
    auto& SD = CSd::getInstance();
    try {
        SD.mInit();
    }
    catch (const std::runtime_error& e) {
        std::printf("Initializing SD threw error: %s\n", e.what());
    }

    auto& log = CLog::getInstance();
    log.mInit();

    auto logger = log.mScope("app.init");
    logger.mDebug("App is being inited");

    auto& sensorManager = CSensorManager::getInstance();
    // init PEAP network
    try {
        CWifi::getInstance().mInitWifi({
            .ssid = "",
            .eapId = "",
            .eapUsername = "",
            .password = ""
        });
    }
    catch (const std::runtime_error &e) {
        logger.mError("Error thrown while initing wifi: %s", e.what());
    }
    
    logger.mDebug("Adding sensors...");
    sensorManager.mAddSensor(new CDbSensor("dbSensor"));
    logger.mDebug("Sensors added, app is inited!");
}

void App::loop() {
    auto& sensorManager = CSensorManager::getInstance();
    auto logger = CLog::getInstance().mScope("app.loop");
    logger.mInfo("Hello World");

    auto sensors = sensorManager.mMeasure();

    // Print all measurements
    for (auto const &sensor: sensors) {
        logger.mInfo("Sensor '%s':", sensor.first.c_str());
        for (auto const &measurement : sensor.second) {
            logger.mInfo("\t%s\t: %s",
                measurement.first.c_str(),
                measurement.second.c_str()
            );
        }
        logger.mInfo("");
    }

    // Throw debug error
    logger.mThrow("If you see this, everything works!");
}

void App::start() {
    try {
        this->init();
        while (true) this->loop();
    }
    catch (const std::runtime_error& e) {
        std::printf(
            "Ah shucks!\n"
            "FATAL unhandled runtime exception occured!\n"
            "Famous lasts words:\n"
            "%s\n",
            e.what()
        );
    }
}
