#pragma once

#include <sensor.hpp>

class CParticleSensor : public CSensor{
    public:
        using CSensor::CSensor;

    private:
        float m_offset = 0.0f;

        SensorOutput m_MeasureCallback();
        CSensorStatus m_InitCallback();
};