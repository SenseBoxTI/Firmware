#include "sensor.hpp"

SensorOutput CSensor::measure() {
    m_LastMeasurement = measureCallback();
    return m_LastMeasurement;
}

CSensorStatus CSensor::init() {
    m_Status = initCallback();
    return m_Status;
}

CSensor::CSensor(std::string aName) {
    mName = aName;
    m_Cooldown = 0.0f;
}