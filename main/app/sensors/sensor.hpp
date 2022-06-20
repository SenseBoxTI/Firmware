#pragma once

#include "sensorstatus.hpp"
#include "sensoroutput.hpp"

#include <CTimer.hpp>
#include <vector>

/**
 * Sensor class
 * Overload the following class when implementing sensors
 */
class CSensor {
    public:
    std::string mName;
    // Calls InitCallback
    SensorOutput mGetResults();
    // Calls MeasureCallback
    CSensorStatus mInit();
    CSensor(std::string aName);
    ~CSensor();

    private:
    CSensorStatus m_Status;
    // This callback is ran when the sensor is initialized
    // It uses CSensorStatus to specify whether the sensor is set up correctly or not.
    virtual CSensorStatus m_InitCallback() = 0;
    // This callback is ran when measurements are requested
    // It uses the SensorOutput to return a JSON-like structure for this device's measurements.
    virtual SensorOutput m_MeasureCallback() = 0;
    static void m_ReadSensor(void* aArg);

    // Every sensor contains a timer which calls the mGetResults() function.
    CTimer* m_MeasureTimer;
    // A total output, this contains the total of the measurements, used when calculating the average
    SensorOutput m_MeasurementTotal;
    // The amount of times that was measured before results were requested, used when calculating the average
    uint32_t m_MeasurementCnt;

    protected:
    // The time between measurements, set via the init callback.
    uint64_t m_MeasureInterval;
};