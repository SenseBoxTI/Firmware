#include "o2sensor.hpp"

#include <config.hpp>
#include <esp_adc_cal.h>
#include <logscope.hpp>

static CLogScope logger{"o2sensor"};

void AdcPrintFeatures();
void AdcPrintCharacteristics(esp_adc_cal_value_t aValType);

void CO2Sensor::m_InitADC() {
    AdcPrintFeatures();

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(static_cast<adc1_channel_t>(ADC_CHANNEL_6), ADC_ATTEN_11db);

    m_adcCharacteristics = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t valueType = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_BIT_12, 1100, static_cast<esp_adc_cal_characteristics_t*>(m_adcCharacteristics));

    AdcPrintCharacteristics(valueType);
}

uint32_t CO2Sensor::m_SampleADC() {
    const uint8_t samples = 64;
    uint32_t reading = 0;

    for (uint8_t i = 0; i < samples; i++) {
        reading += adc1_get_raw(static_cast<adc1_channel_t>(ADC_CHANNEL_6));
    }

    reading /= samples;

    return esp_adc_cal_raw_to_voltage(reading, static_cast<esp_adc_cal_characteristics_t*>(m_adcCharacteristics));
}

SensorOutput CO2Sensor::m_MeasureCallback() {
    SensorOutput output;
    // voltage range 0 - 3089, see O2 doc
    // sensor range 0 - 25%
    const float ratio = 25.0f / 3089.0f;

    static char sampleString[8] = { 0 };
    float measurement = (static_cast<float>(m_SampleADC()) * m_rc * ratio) + m_offset;
    std::snprintf(sampleString, 8, "%.4f", measurement);
    output.emplace("O2%", sampleString);

    return output;
}

CSensorStatus CO2Sensor::m_InitCallback() {
    m_InitADC();

    auto& calibration = CConfig::getInstance()["calibration"];

    if (calibration.valid()) {
        auto& o2SensorCalibration = calibration["o2"];

        if (o2SensorCalibration.valid()) {
            m_rc = o2SensorCalibration.get<double>("rc");
            m_offset = o2SensorCalibration.get<double>("offset");
        }
    }

    return CSensorStatus::Ok();
}

void AdcPrintFeatures() {
        logger.mInfo("ADC Feature     | Supported");
        logger.mInfo("                |          ");
        logger.mInfo("eFuse two point | %s", esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) ? "No" : "Yes");
        logger.mInfo("eFuse Vref      | %s", esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) ? "No" : "Yes");
}

void AdcPrintCharacteristics(esp_adc_cal_value_t aValType) {
    logger.mInfo(
        "Characterized by using %s",
        (aValType == ESP_ADC_CAL_VAL_EFUSE_TP) ? "Two Point Value" :
        (aValType == ESP_ADC_CAL_VAL_EFUSE_VREF) ? "eFuse Vref" :
        "Default Vref"
    );
}