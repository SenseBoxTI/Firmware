#include "MAX4466.hpp"
#include <esp_adc_cal.h>
#include <config.hpp>
#include <CConfig.hpp>

uint32_t CMax4466::m_SampleADC() {
    uint32_t reading = 0;
    uint32_t highest_sample = 0;
    uint32_t lowest_sample = 4049;
    int64_t startMillis = esp_timer_get_time();

    // sample every 50ms aka 20hz (lowest hz a human can hear)
    while (esp_timer_get_time() - startMillis < 50000) {
        reading = adc1_get_raw(static_cast<adc1_channel_t>(ADC_CHANNEL_7));

        if (reading > highest_sample) {
            highest_sample = reading;
        } else if (reading < lowest_sample) {
            lowest_sample = reading;
        }
    }
    // calculate average of the 20hz
    uint32_t peakToPeak = highest_sample - lowest_sample;
    //     voltage = ((average peak * 3.3v) / ADC channel) * magic number
    double voltage = ((peakToPeak * 3.3) / 4048) * 0.707;
    //       result = log10( voltage / rc) * magic number) - mic sensitivity + magic number - amp gain
    uint32_t result = (log10(voltage / m_rc) * 20) - 44 + 94 - 25;
//                                        0.00631  0.005012
    return result;
}

SensorOutput CMax4466::m_MeasureCallback() {
    SensorOutput output;

    float measurement = (static_cast<float>(m_SampleADC()) * m_rc) + m_offset;
    output.emplace("dB", measurement);

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
            printf("%f", m_rc);
            printf("%f", m_offset);
        }
    }

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_BIT_12, 1100, static_cast<esp_adc_cal_characteristics_t*>(m_AdcCharacteristics));
    return CSensorStatus::Ok();
}
