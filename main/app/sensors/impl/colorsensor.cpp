#include "colorsensor.hpp"

#include <Adafruit_AS726x.h>
#include <Wire.h>

Adafruit_AS726x ams;

float calibratedValues[AS726x_NUM_CHANNELS];
float offsets[AS726x_NUM_CHANNELS] = {0};

constexpr const char* colors[] = {
    "MAGENTA",
    "BLUE",
    "GREEN",
    "YELLOW",
    "ORANGE",
    "RED"};

enum ColorEnum {
    MAGENTA,
    BLUE,
    GREEN,
    YELLOW,
    ORANGE,
    RED
};

SensorOutput CColorSpectrumSensor::m_MeasureCallback() {
    SensorOutput output;
    if (ams.dataReady()) {
        ams.readCalibratedValues(calibratedValues);

        for (int i = 0; i < AS726x_NUM_CHANNELS; i++) {
            output.insert({colors[i], std::to_string(calibratedValues[i] + offsets[i])});
        }
    } else {
        for (int i = 0; i < AS726x_NUM_CHANNELS; i++) {
            output.insert({colors[i], "NaN"});
        }
    }
    return output;
}

CSensorStatus CColorSpectrumSensor::m_InitCallback() {
    if (!Wire.begin(47, 48)) {
        return CSensorStatus::Error("TwoWire failed to init!");
    }

    if (!ams.begin(&Wire)) {
        return CSensorStatus::Error("Could not begin color spectrum sensor!");
    }

    ams.setConversionType(MODE_2);

    offsets[MAGENTA] = 0.0f;
    offsets[BLUE] = 0.0f;
    offsets[GREEN] = 0.0f;
    offsets[YELLOW] = 0.0f;
    offsets[ORANGE] = 0.0f;
    offsets[RED] = 0.0f;

    return CSensorStatus::Ok();
}
