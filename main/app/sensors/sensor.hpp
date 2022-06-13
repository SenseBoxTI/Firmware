#pragma once

#include "sensorstatus.hpp"
#include "sensoroutput.hpp"

#include <esp_timer.h>
#include <vector>

class CSensor {
    public:
    std::string mName;
    SensorOutput mGetResults();
    CSensorStatus mInit();
    void mClearTask();
    CSensor(std::string aName);

    private:
    CSensorStatus m_Status;
    virtual CSensorStatus m_InitCallback() = 0;
    virtual SensorOutput m_MeasureCallback() = 0;
    static void m_ReadSensor(void* aArg);

    esp_timer_handle_t m_MeasureTimer;
    SensorOutput m_MeasurementTotal;
    uint32_t m_MeasurementCnt;

    protected:
    uint64_t m_MeasureInterval;
};