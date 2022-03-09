#pragma once

#include <sensor.hpp>

class CDbSensor : public CSensor {
    public:
    using CSensor::CSensor;

    private:
    SensorOutput measureCallback();
    CSensorStatus initCallback();
};