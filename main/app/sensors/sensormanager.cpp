#include "sensormanager.hpp"

CSensorManager& CSensorManager::getInstance() {
    static CSensorManager instance = {};
    return instance;
}

CSensorManager::CSensorManager() {
}

CSensorStatus CSensorManager::addSensor(CSensor* apSensor) {
    auto status = apSensor->init();
    m_Sensors.push_back(apSensor);
    return status;
}

Measurements CSensorManager::measure() {
    Measurements measurements;

    for (auto sensor : m_Sensors) {
        measurements.insert({sensor->mName, sensor->measure()});
    }
    m_Measurements = measurements;

    return m_Measurements;
}