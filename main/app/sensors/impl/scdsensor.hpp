#pragma once

#include <sensor.hpp>

class CScdSensor : public CSensor {
    public:
        using CSensor::CSensor;
        
        [[nodiscard]] float get_LastTemperature() { return m_LastTemperature; }
        [[nodiscard]] float get_LastRelative_humidity() { return m_LastRelative_humidity; }
        [[nodiscard]] float get_LastCO2() { return m_LastCO2; }
    private:
        float m_LastTemperature = 0.0f;
        float m_LastRelative_humidity = 0.0f;
        float m_LastCO2 = 0.0f;

        SensorOutput m_MeasureCallback();
        CSensorStatus m_InitCallback();
};
