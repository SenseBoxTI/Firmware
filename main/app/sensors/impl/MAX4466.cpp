#include "MAX4466.hpp"
#include <esp_adc_cal.h>
#include <config.hpp>
#include <CConfig.hpp>

uint32_t CMax4466::m_SampleADC() {
    const int samples = 64;
    uint32_t reading = 0;

    for (int i = 0; i < samples; i++) {\
        reading += adc1_get_raw(static_cast<adc1_channel_t>(ADC_CHANNEL_7));
    }

    reading /= samples;

    return esp_adc_cal_raw_to_voltage(reading, static_cast<esp_adc_cal_characteristics_t*>(m_AdcCharacteristics));
}

SensorOutput CMax4466::m_MeasureCallback() {
    SensorOutput output;

    float measurement = (static_cast<float>(m_SampleADC()) * m_rc) + m_offset;
    output.emplace("db", measurement);

    return output;
}

CSensorStatus CMax4466::m_InitCallback() {
    m_MeasureInterval = MAX4466_MEASURE_INTERVAL_US;

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(static_cast<adc1_channel_t>(ADC_CHANNEL_7), ADC_ATTEN_11db);

    m_AdcCharacteristics = calloc(1, sizeof(esp_adc_cal_characteristics_t));

    auto& calibration = CConfig::getInstance()["calibration"];

    if (calibration.valid()) {
        auto& dbSensorCalibration = calibration["db"];

        if (dbSensorCalibration.valid()) {
            m_rc = dbSensorCalibration.get<double>("rc");
            m_offset = dbSensorCalibration.get<double>("offset");
        }
    }

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_BIT_12, 1100, static_cast<esp_adc_cal_characteristics_t*>(m_AdcCharacteristics));
    return CSensorStatus::Ok();
}
