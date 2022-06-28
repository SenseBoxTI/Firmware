#pragma once

#include "sensor.hpp"
#include "measurements.hpp"
#include <vector>

class CSensorManager {
public:
    static CSensorManager& getInstance();
    CSensorStatus mAddSensor(CSensor* apSensor);
    Measurements mGetResults();
    void mReinit();

private:
    std::vector<CSensor*> m_Sensors;
    Measurements m_Measurements;
    CSensorManager();
};