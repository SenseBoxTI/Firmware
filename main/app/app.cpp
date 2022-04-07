#include "app.hpp"
#include <stdexcept>
#include <iostream>
#include <sensormanager.hpp>
#include <adctest.hpp>
#include <i2ctest.hpp>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void App::init() {
    std::printf("Sensebox PCB Test Application\n");
    auto& sensorManager = CSensorManager::getInstance();

    sensorManager.mAddSensor(new CAdcTest("ADC Test"));
    sensorManager.mAddSensor(new CI2cTest("I2C Test"));
}

void App::loop() {
    auto& sensorManager = CSensorManager::getInstance();

    auto sensors = sensorManager.mMeasure();

   std::printf("Sensor evaluation:\n");
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
    // throw std::runtime_error("If you see this, everything works!\n");
    vTaskDelay(2000 / portTICK_RATE_MS);
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
