#include "app.hpp"
#include <stdexcept>
#include <iostream>
#include <sensormanager.hpp>
#include <dbsensor.hpp>
#include <wifi.hpp>
#include <file.hpp>
#include <sd.hpp>
#include <sys/unistd.h>


void App::init() {
    std::printf("app.init()\n");
    auto& sensorManager = CSensorManager::getInstance();
    CFile testfile = CFile("/sdcard/sd_test.txt");
    // init PEAP network
    
    // try {
    //     CWifi::getInstance().mInitWifi({
    //         .ssid = "",
    //         .eapId = "",
    //         .eapUsername = "",
    //         .password = ""
    //     });
    // }
    // catch (const std::runtime_error &e) {
    //     std::printf("Error thrown while initing wifi: %s", e.what());
    // }
    
    sensorManager.mAddSensor(new CDbSensor("dbSensor"));
    auto& SD = CSD::getInstance();
    auto ret = SD.mInit(); 
    if (ret == ESP_OK) {
        SD.mGetFile("/sdcard/sd_test.txt");
        testfile.mWrite("Banaan");
        testfile.mRead();
            } else {
        std::printf("Failed to init SD: %s\n", esp_err_to_name(ret));
    }
    // CSD::mGetFile("/sdcard/sd_test.txt");
    // CFile::mWrite("Banaan");

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
