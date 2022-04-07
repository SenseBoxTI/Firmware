#pragma once

#include <sensor.hpp>

class CAdcTest : public CSensor {
    public:
    using CSensor::CSensor;

    private:
    void * m_adcCharacteristics = nullptr;
    void m_InitADC();
    uint32_t m_SampleADC(bool primary);

    SensorOutput m_MeasureCallback();
    CSensorStatus m_InitCallback();
};