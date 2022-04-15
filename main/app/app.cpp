#include "app.hpp"
#include <stdexcept>
#include <iostream>
#include <sensormanager.hpp>
#include <dbsensor.hpp>
#include <o2sensor.hpp>
#include <wifi.hpp>
#include <file.hpp>

void App::init() {
    std::printf("app.init()\n");
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
        std::printf("Error thrown while initing wifi: %s\n", e.what());
    }

    sensorManager.mAddSensor(new CO2Sensor("O2Sensor"));
    sensorManager.mAddSensor(new CDbSensor("dbSensor"));

    try {
        CFile::mInitSd();
    }
    catch (const std::runtime_error& e) {
        std::printf("Initializing SD threw error: %s", e.what());
    }
}

void App::loop() {
    std::printf("app.loop()\n");
    auto& sensorManager = CSensorManager::getInstance();

    auto sensors = sensorManager.mMeasure();

    // Print all measurements
    for (auto const &sensor: sensors) {
        std::printf("Sensor '%s':\n", sensor.first.c_str());
        for (auto const &measurement : sensor.second) {
            std::printf("\t%s\t: %s\n",
                measurement.first.c_str(),
                measurement.second.c_str()
            );
        }
        std::printf("\n");
    }
    // Throw debug error
    throw std::runtime_error("If you see this, everything works!\n");
}

void App::start() {
    try {
        this->init();
        while (true) this->loop();
    }
    catch (const std::runtime_error &e) {
        std::printf(
            "Ah shucks!\n"
            "FATAL unhandled runtime exception occured!\n"
            "Famous lasts words:\n"
            "%s",
            e.what()
        );
    }
}
