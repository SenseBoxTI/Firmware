#include "dbsensor.hpp"

SensorOutput CDbSensor::measureCallback() {
    SensorOutput output;

    output.insert({"db", "40.0"});

    return output;
}

CSensorStatus CDbSensor::initCallback() {
    return CSensorStatus::Ok();
}