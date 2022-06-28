#pragma once

#include <sensor.hpp>

class CMix8410 : public CSensor {
    public:
    using CSensor::CSensor;

    private:
    void * m_adcCharacteristics = nullptr;
    float m_rc = 1.0f;
    float m_offset = 0.0f;

    void m_InitADC();
    uint32_t m_SampleADC();
    SensorOutput m_MeasureCallback();
    CSensorStatus m_InitCallback();
};