#pragma once

#include <sensor.hpp>

class CLightIntensitySensor : public CSensor {
    public:
    using CSensor::CSensor;

    private:
    float m_offset;
    SensorOutput m_MeasureCallback();
    CSensorStatus m_InitCallback();
};
