#pragma once

#include "sensor.hpp"
#include "measurements.hpp"
#include <vector>

class CSensorManager {
public:
    static CSensorManager& getInstance();
    CSensorStatus mAddSensor(CSensor* apSensor);
    Measurements mMeasure();

private:
    std::vector<CSensor*> m_Sensors;
    Measurements m_Measurements;
    CSensorManager();
};