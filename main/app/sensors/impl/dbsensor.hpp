#pragma once

#include <sensor.hpp>

class CDbSensor : public CSensor {
    public:
    using CSensor::CSensor;

    private:
    void* m_AdcCharacteristics = nullptr;
    float m_rc = 1.0f;
    float m_offset = 0.0f;

    uint32_t m_SampleADC();
    SensorOutput m_MeasureCallback();
    CSensorStatus m_InitCallback();
};
