#pragma once

#include <sensor.hpp>

class CDbSensor : public CSensor {
    public:
    using CSensor::CSensor;

    private:
    void * m_AdcCharacteristics = nullptr;

    uint32_t m_SampleADC();
    SensorOutput m_MeasureCallback();
    CSensorStatus m_InitCallback();
};
