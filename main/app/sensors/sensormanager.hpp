#pragma once

#include "sensor.hpp"
#include "measurements.hpp"
#include <vector>

/**
 * Sensor Manager class
 * This handles the registration, initialization and measuring of all sensors
 * It helps in creating a simple system in which new hardware can easily be implemented
 * through a standardized model.
 */
class CSensorManager {
public:
    static CSensorManager& getInstance();
    // Add a sensor
    CSensorStatus mAddSensor(CSensor* apSensor);
    // Retrieve results based on averages from measurements
    Measurements mGetResults();

private:
    std::vector<CSensor*> m_Sensors;
    Measurements m_Measurements;
    CSensorManager();
};