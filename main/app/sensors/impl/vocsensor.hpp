#pragma once

#include <sensor.hpp>
#include <scdsensor.hpp>

class CVocSensor : public CSensor {
    public:
        CVocSensor(std::string aName, CScdSensor& arScdSensor);

    private:
        float m_factor = 1.0f;

        CScdSensor& mr_ScdSensor;
        SensorOutput m_MeasureCallback();
        CSensorStatus m_InitCallback();
};
