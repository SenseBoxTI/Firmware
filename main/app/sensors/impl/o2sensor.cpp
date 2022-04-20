#include "o2sensor.hpp"

#include <esp_adc_cal.h>

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

    // TODO config file implementation

    static char sampleString[8] = { 0 };
    std::snprintf(sampleString, 8, "%.4f", static_cast<float>(m_SampleADC()) * ratio);

    output.insert({"O2%", sampleString});

    return output;
}

CSensorStatus CO2Sensor::m_InitCallback() {
    m_InitADC();
    return CSensorStatus::Ok();
}

void AdcPrintFeatures() {
    std::printf(
        "ADC Feature     | Supported\n"
        "                |          \n"
        "eFuse two point | %s\n"
        "eFuse Vref      | %s\n"
        "\n",
        esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) ? "No" : "Yes",
        esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) ? "No" : "Yes"
    );
}

void AdcPrintCharacteristics(esp_adc_cal_value_t aValType) {
    std::printf(
        "Characterized by using %s\n",
        (aValType == ESP_ADC_CAL_VAL_EFUSE_TP) ? "Two Point Value" :
        (aValType == ESP_ADC_CAL_VAL_EFUSE_VREF) ? "eFuse Vref" :
        "Default Vref"
    );
}