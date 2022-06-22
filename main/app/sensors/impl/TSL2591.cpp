#include "TSL2591.hpp"
#include <config.hpp>
#include <Adafruit_TSL2591.h>
#include <CConfig.hpp>

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

SensorOutput CTsl2591::m_MeasureCallback() {
    SensorOutput output;

    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir = lum >> 16;
    uint16_t full = lum & 0xFFFF;
    uint16_t visible = full - ir;
    float lux = tsl.calculateLux(full, ir) + m_offset;

    output.emplace("ir", static_cast<float>(ir));
    output.emplace("full", static_cast<float>(full));
    output.emplace("visible", static_cast<float>(visible));
    output.emplace("lux", lux);

    return output;
}

CSensorStatus CTsl2591::m_InitCallback() {
    m_MeasureInterval = TSL2591_MEASURE_INTERVAL_US;

    if (!Wire.begin(47, 48)) {
        return CSensorStatus::Error("TwoWire failed to init!");
    }

    if (!tsl.begin(&Wire)) {
        return CSensorStatus::Error("Could not begin light intensity sensor!");
    }

    tsl.setGain(TSL2591_GAIN_MED);
    tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);

    auto& calibration = CConfig::getInstance()["calibration"];

    if (calibration.valid()) {
        auto& lightSensorOffsets = calibration["lightintensity"];

        if (lightSensorOffsets.valid()) {
            m_offset = lightSensorOffsets.get<double>("offset");
        }
    }

    return CSensorStatus::Ok();
}
