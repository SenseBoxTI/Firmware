#include "sensor.hpp"

SensorOutput CSensor::mGetResults() {
    m_LastMeasurement = m_GetResultsCallback();
    return m_LastMeasurement;
}

CSensorStatus CSensor::mInit() {
    m_Status = m_InitCallback();
    return m_Status;
}

CSensor::CSensor(std::string aName) {
    mName = aName;
}