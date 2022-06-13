#pragma once

#include "sensorstatus.hpp"
#include "sensoroutput.hpp"

class CSensor {
    public:
    std::string mName;
    SensorOutput mGetResults();
    CSensorStatus mInit();
    CSensor(std::string aName);

    private:
    SensorOutput m_LastMeasurement;
    CSensorStatus m_Status;
    virtual CSensorStatus m_InitCallback() = 0;
    virtual SensorOutput m_GetResultsCallback() = 0;
};