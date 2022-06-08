#include "lightintensitysensor.hpp"

#include <Adafruit_TSL2591.h>

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

SensorOutput CLightIntensitySensor::m_MeasureCallback() {
    SensorOutput output;
    
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir = lum >> 16;
    uint16_t full = lum & 0xFFFF;
    uint16_t visible = full - ir;
    float lux = tsl.calculateLux(full, ir) + m_offset;

    output.insert({"ir", std::to_string(ir)});
    output.insert({"full", std::to_string(full)});
    output.insert({"visible", std::to_string(visible)});
    output.insert({"lux", std::to_string(lux)});

    return output;
}

CSensorStatus CLightIntensitySensor::m_InitCallback() {
    if (!Wire.begin(47, 48)) {
        return CSensorStatus::Error("TwoWire failed to init!");
    }

    if (!tsl.begin(&Wire)) {
        return CSensorStatus::Error("Could not begin light intensity sensor!");
    }

    tsl.setGain(TSL2591_GAIN_MED);
    tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);

    // configure lux offset
    m_offset = 0.0f;

    return CSensorStatus::Ok();
}
