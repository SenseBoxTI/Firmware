#pragma once

#include <sensor.hpp>

class CPmsa0003I : public CSensor{
    public:
        using CSensor::CSensor;

    private:
        float m_factor = 1.0f;
        
        SensorOutput m_MeasureCallback();
        CSensorStatus m_InitCallback();
};