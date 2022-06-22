#pragma once

#include <sensor.hpp>

class CAS726x : public CSensor {
public:
    using CSensor::CSensor;

private:
    SensorOutput m_MeasureCallback();
    CSensorStatus m_InitCallback();
};
