#pragma once

#include <sensor.hpp>

class CWifiTest : public CSensor {
    public:
    using CSensor::CSensor;

    private:
    SensorOutput m_MeasureCallback();
    CSensorStatus m_InitCallback();
};