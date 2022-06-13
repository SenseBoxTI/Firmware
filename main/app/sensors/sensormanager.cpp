#include "sensormanager.hpp"

CSensorManager& CSensorManager::getInstance() {
    static CSensorManager instance = {};
    return instance;
}

CSensorManager::CSensorManager() {
}

CSensorStatus CSensorManager::mAddSensor(CSensor* apSensor) {
    auto status = apSensor->mInit();
    m_Sensors.push_back(apSensor);
    return status;
}

Measurements CSensorManager::mGetResults() {
    Measurements measurements;

    for (auto& sensor : m_Sensors) {
        measurements.emplace(sensor->mName, sensor->mGetResults());
    }
    m_Measurements = measurements;

    return m_Measurements;
}

void CSensorManager::mClearTasks() {
    for (auto& sensor : m_Sensors) {
        sensor->mClearTask();
    }
}
