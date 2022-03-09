#include "dbsensor.hpp"

SensorOutput CDbSensor::m_MeasureCallback() {
    SensorOutput output;

    output.insert({"db", "40.0"});

    return output;
}

CSensorStatus CDbSensor::m_InitCallback() {
    return CSensorStatus::Ok();
}