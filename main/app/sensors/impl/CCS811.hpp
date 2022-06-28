#pragma once

#include <sensor.hpp>
#include <SCD30.hpp>

class CCcs811 : public CSensor {
    public:
        CCcs811(std::string aName, CScd30& arScdSensor);

    private:
        float m_factor = 1.0f;

        CScd30& mr_ScdSensor;
        SensorOutput m_MeasureCallback();
        CSensorStatus m_InitCallback();
};
