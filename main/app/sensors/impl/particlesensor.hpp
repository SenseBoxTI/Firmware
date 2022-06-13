#pragma once

#include <sensor.hpp>

class CParticleSensor : public CSensor{
    public:
        using CSensor::CSensor;

    private:
        SensorOutput m_MeasureCallback();
        CSensorStatus m_InitCallback();
};