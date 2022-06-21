#include "colorsensor.hpp"
#include <config.hpp>

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

    auto& calibration = CConfig::getInstance()["calibration"];

    if (calibration.valid()) {
        auto& colorSensorOffsets = calibration["colorspectrum"];

        if (colorSensorOffsets.valid()) {
            offsets[0] = colorSensorOffsets.get<double>("magentaOffset"); // MAGENTA
            offsets[1] = colorSensorOffsets.get<double>("blueOffset"); // BLUE
            offsets[2] = colorSensorOffsets.get<double>("greenOffset"); // GREEN
            offsets[3] = colorSensorOffsets.get<double>("yellowOffset"); // YELLOW
            offsets[4] = colorSensorOffsets.get<double>("orangeOffset"); // ORANGE
            offsets[5] = colorSensorOffsets.get<double>("redOffset"); // RED
        }
    }

    return CSensorStatus::Ok();
}
