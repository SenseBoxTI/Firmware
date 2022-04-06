#pragma once

#include <sensor.hpp>

class CI2cTest : public CSensor {
    public:
    using CSensor::CSensor;

    private:
    SensorOutput m_MeasureCallback();
    CSensorStatus m_InitCallback();
};