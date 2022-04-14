#include "adctest.hpp"

#include <esp_adc_cal.h>

void AdcPrintFeatures();
void AdcPrintCharacteristics(esp_adc_cal_value_t aValType);

void CAdcTest::m_InitADC() {
    AdcPrintFeatures();

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(static_cast<adc1_channel_t>(ADC_CHANNEL_7), ADC_ATTEN_11db);
    adc1_config_channel_atten(static_cast<adc1_channel_t>(ADC_CHANNEL_2), ADC_ATTEN_11db);

    m_adcCharacteristics = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t valueType = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_BIT_12, 1100, static_cast<esp_adc_cal_characteristics_t*>(m_adcCharacteristics));

    AdcPrintCharacteristics(valueType);
}

uint32_t CAdcTest::m_SampleADC(bool primary) {
    const int samples = 64;
    uint32_t reading = 0;

    for (int i = 0; i < samples; i++) {\
        reading += adc1_get_raw(static_cast<adc1_channel_t>((primary) ? ADC_CHANNEL_7 : ADC_CHANNEL_2));
    }

    reading /= samples;

    return esp_adc_cal_raw_to_voltage(reading, static_cast<esp_adc_cal_characteristics_t*>(m_adcCharacteristics));
}

SensorOutput CAdcTest::m_MeasureCallback() {
    SensorOutput output;

    static char sampleString[16] = { 0 };
    std::snprintf(sampleString, 16, "%d", m_SampleADC(true));
    output.insert({"mV (AUDIO)", sampleString});
    std::snprintf(sampleString, 16, "%d", m_SampleADC(false));
    output.insert({"mV (O2)", sampleString});

    return output;
}

CSensorStatus CAdcTest::m_InitCallback() {
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