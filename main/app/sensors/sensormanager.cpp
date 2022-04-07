#include "sensormanager.hpp"

CSensorManager& CSensorManager::getInstance() {
    static CSensorManager instance = {};
    return instance;
}

CSensorManager::CSensorManager() {
}

CSensorStatus CSensorManager::mAddSensor(CSensor* apSensor) {
    auto status = apSensor->mInit();
    std::string reason = "";

    if (!status.mIsOk(reason)) {
        printf("Sensor '%s' failed to initialize.\nReason: %s\n", apSensor->mName.c_str(), reason.c_str());
    }
    else {
        m_Sensors.push_back(apSensor);
    }

    return status;
}

Measurements CSensorManager::mMeasure() {
    Measurements measurements;

    for (auto sensor : m_Sensors) {
        measurements.insert({sensor->mName, sensor->mMeasure()});
    }
    m_Measurements = measurements;

    return m_Measurements;
}