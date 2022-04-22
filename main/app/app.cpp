#include "app.hpp"
#include <stdexcept>
#include <iostream>
#include <sensormanager.hpp>
#include <dbsensor.hpp>
#include <o2sensor.hpp>
#include <wifi.hpp>
#include <log.hpp>
#include <file.hpp>
#include <time.hpp>

void App::init() {
    // Initialize Logger
    auto& log = CLog::getInstance();
    log.mInit();

    auto logger = log.mScope("app.init");
    logger.mDebug("Application is starting");

    logger.mInfo("Initializing SD");
    // Initialize SD
    try {
        CFile::mInitSd();
    }
    catch (const std::runtime_error& e) {
        logger.mError("Initializing SD threw error: %s", e.what());
    }

    logger.mInfo("Initializing Wifi");
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

    logger.mInfo("Initializing Time");
    try {
        CTime::mInitTime("pool.ntp.org");
    }
    catch (const std::runtime_error& e) {
        logger.mError("Initializing NTP threw error: %s", e.what());
    }

    auto& sensorManager = CSensorManager::getInstance();

    logger.mDebug("Adding sensors...");
    sensorManager.mAddSensor(new CDbSensor("dbSensor"));
    sensorManager.mAddSensor(new CO2Sensor("O2Sensor"));

    logger.mInfo("System has started.\nThe current time is: %s\n\n", CTime::mGetTimeString().c_str());
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

    throw std::runtime_error("If you see this, everything works!");
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
