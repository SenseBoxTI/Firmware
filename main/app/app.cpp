#include "app.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sensormanager.hpp>
#include <dbsensor.hpp>
#include <sd.hpp>

#include <sys/unistd.h>
// temp var to keep track of cycle count
uint64_t dataTestCounter = 0;

void App::init() {
    std::printf("app.init()\n");
    auto& sensorManager = CSensorManager::getInstance();
    sensorManager.mAddSensor(new CDbSensor("dbSensor"));

    auto& SD = CSD::getInstance();
    auto ret = SD.mInit();
    if (ret == ESP_OK) {
        auto file = fopen(MOUNT_POINT "/sd_test.txt", "w");
        fprintf(file, "Testing file write\n");
        fclose(file);
    } else {
        std::printf("Failed to init SD: %s\n", esp_err_to_name(ret));
    }
}

void App::loop() {
    std::printf("app.loop()\n");
    auto& sensorManager = CSensorManager::getInstance();

    auto sensors = sensorManager.mMeasure();
    // Print all measurements
    auto file = fopen(MOUNT_POINT "/sd_test.txt", "a");
    fprintf(file, "-- data %llu --\n", dataTestCounter);
    dataTestCounter++;
    for (auto const &sensor: sensors) {
        std::printf("Sensor '%s':\n", sensor.first.c_str());

        for (auto const &measurement : sensor.second) {
            std::printf("\t%s\t: %s\n",
                measurement.first.c_str(),
                measurement.second.c_str()
            );

            if (file != nullptr) {
                fprintf(file, "\t%s\t: %s\n",
                    measurement.first.c_str(),
                    measurement.second.c_str()
                    );
            }
        }

        std::printf("\n");
    }

    fprintf(file, "---------------\n");
    fclose(file);

    std::printf("Printing out sd_test.txt:\n");
    
    std::ifstream file_read(MOUNT_POINT "/sd_test.txt", std::ios::out | std::ios::binary);
    char c = file_read.get();
    while (file_read.good()) {
        std::printf("%c", c);
        c = file_read.get();
    } 

    file_read.close();
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