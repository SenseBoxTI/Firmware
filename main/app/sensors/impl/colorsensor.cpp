#include "colorsensor.hpp"

#include <Adafruit_AS726x.h>
#include <Wire.h>

#include <CConfig.hpp>

Adafruit_AS726x as726x;

float calibratedValues[AS726x_NUM_CHANNELS];
float offsets[AS726x_NUM_CHANNELS];

constexpr const char* colors[] = {
    "MAGENTA",
    "BLUE",
    "GREEN",
    "YELLOW",
    "ORANGE",
    "RED"
};

SensorOutput CColorSpectrumSensor::m_MeasureCallback() {
    SensorOutput output;

    if (!as726x.dataReady()) return output;

    as726x.readCalibratedValues(calibratedValues);

    for (int i = 0; i < AS726x_NUM_CHANNELS; i++) {
        output.emplace(colors[i], calibratedValues[i] + offsets[i]);
    }

    return output;
}

CSensorStatus CColorSpectrumSensor::m_InitCallback() {
    m_MeasureInterval = AS7262_MEASURE_INTERVAL_US;

    if (!Wire.begin(47, 48)) {
        return CSensorStatus::Error("TwoWire failed to init!");
    }

    if (!as726x.begin(&Wire)) {
        return CSensorStatus::Error("Could not begin color spectrum sensor!");
    }

    as726x.setConversionType(MODE_2);

    offsets[0] = 0.0f; // MAGENTA
    offsets[1] = 0.0f; // BLUE
    offsets[2] = 0.0f; // GREEN
    offsets[3] = 0.0f; // YELLOW
    offsets[4] = 0.0f; // ORANGE
    offsets[5] = 0.0f; // RED

    return CSensorStatus::Ok();
}
