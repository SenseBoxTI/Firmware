#include "sensor.hpp"

SensorOutput CSensor::mMeasure() {
    m_LastMeasurement = m_MeasureCallback();
    return m_LastMeasurement;
}

CSensorStatus CSensor::mInit() {
    m_Status = m_InitCallback();
    return m_Status;
}

CSensor::CSensor(std::string aName) {
    mName = aName;
    m_Cooldown = 0.0f;
}