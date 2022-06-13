#include "sensor.hpp"

#include <CConfig.hpp>
#include <stdexcept>

SensorOutput CSensor::mGetResults() {
    if (m_MeasurementCnt == 0) {
        throw std::runtime_error("Sensor " + mName + " did not make any measurements within the set interval of " + std::to_string(SEND_INTERVAL_US / 1000000) + " seconds.");
    }

    SensorOutput output;
    for (auto& el : m_MeasurementTotal) output[el.first] = el.second / m_MeasurementCnt;

    m_MeasurementCnt = 0;
    m_MeasurementTotal.clear();

    return output;
}

CSensorStatus CSensor::mInit() {
    m_Status = m_InitCallback();

    if (m_MeasureInterval > SEND_INTERVAL_US || m_MeasureInterval < 10000) {
        throw std::runtime_error("Measure interval must be shorter than " + std::to_string(SEND_INTERVAL_US / 1000000) + "s and longer than 10ms");
    }

    esp_timer_create_args_t createArgs = {
        .callback = &m_ReadSensor,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = mName.c_str()
    };

    ESP_ERROR_CHECK(esp_timer_create(&createArgs, &m_MeasureTimer));

    ESP_ERROR_CHECK(esp_timer_start_periodic(m_MeasureTimer, m_MeasureInterval));

    return m_Status;
}

void CSensor::m_ReadSensor(void* aSelf) {
    CSensor& self = *static_cast<CSensor*>(aSelf);
    SensorOutput measurement = self.m_MeasureCallback();

    if (measurement.size() == 0) return;

    for (auto& el : measurement) {
        self.m_MeasurementTotal[el.first] += el.second;
    }
    self.m_MeasurementCnt++;
}
CSensor::CSensor(std::string aName)
:   m_MeasurementCnt(0),
    m_MeasureInterval(0)
{
    if (aName.length() == 0) throw std::runtime_error("Sensor name is required");

    mName = aName;
}