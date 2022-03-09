#pragma once

#include "sensorstatus.hpp"
#include "sensoroutput.hpp"

class CSensor {
    public:
    std::string mName;
    SensorOutput measure();
    CSensorStatus init();
    CSensor(std::string aName);

    private:
    float m_Cooldown;
    SensorOutput m_LastMeasurement;
    CSensorStatus m_Status;
    virtual CSensorStatus initCallback() = 0;
    virtual SensorOutput measureCallback() = 0;    
};