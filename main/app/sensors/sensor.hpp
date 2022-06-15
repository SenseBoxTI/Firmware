#pragma once

#include "sensorstatus.hpp"
#include "sensoroutput.hpp"

#include <CTimer.hpp>
#include <vector>

class CSensor {
    public:
    std::string mName;
    SensorOutput mGetResults();
    CSensorStatus mInit();
    CSensor(std::string aName);
    ~CSensor();

    private:
    CSensorStatus m_Status;
    virtual CSensorStatus m_InitCallback() = 0;
    virtual SensorOutput m_MeasureCallback() = 0;
    static void m_ReadSensor(void* aArg);

    CTimer* m_MeasureTimer;
    SensorOutput m_MeasurementTotal;
    uint32_t m_MeasurementCnt;

    protected:
    uint64_t m_MeasureInterval;
};