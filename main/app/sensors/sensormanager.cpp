#include "sensormanager.hpp"

#include <logscope.hpp>

static CLogScope logger{"sensorManager"};

CSensorManager& CSensorManager::getInstance() {
    static CSensorManager instance = {};
    return instance;
}

CSensorManager::CSensorManager() {
}

CSensorStatus CSensorManager::mAddSensor(CSensor* apSensor) {
    logger.mInfo("Initializing sensor: %s", apSensor->mName.c_str());
    std::string reason = "";

    auto status = apSensor->mInit();

    if (status.mIsOk(reason)) {
        m_Sensors.push_back(apSensor);
    } else {
        delete apSensor;
        logger.mWarn("Initialization failed: %s", reason.c_str());
    }

    return status;
}

void CSensorManager::mReinit() {
    for (auto& sensor : m_Sensors) {
        sensor->mInit();
    }
}

Measurements CSensorManager::mGetResults() {
    Measurements measurements;

    for (auto& sensor : m_Sensors) {
        measurements.emplace(sensor->mName, sensor->mGetResults());
    }
    m_Measurements = measurements;

    return m_Measurements;
}
