#pragma once

#include <sensor.hpp>

class CScd30 : public CSensor {
    public:
        using CSensor::CSensor;

        [[nodiscard]] float get_Temperature() { return m_LastTemperature; }
        [[nodiscard]] float get_RelativeHumidity() { return m_LastRelative_humidity; }
    private:
        float m_LastTemperature = 0.0f;
        float m_LastRelative_humidity = 0.0f;

        SensorOutput m_MeasureCallback();
        CSensorStatus m_InitCallback();
};
