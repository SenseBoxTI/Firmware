#include "scdsensor.hpp"
#include <Adafruit_SCD30.h>
#include <config.hpp>

Adafruit_SCD30 scd30 = Adafruit_SCD30();

SensorOutput CScdSensor::m_MeasureCallback() {
    SensorOutput output;
    if (scd30.dataReady() && scd30.read()) {
        m_LastTemperature = scd30.temperature;
        m_LastRelative_humidity = scd30.relative_humidity;
        m_LastCO2 = scd30.CO2;

        output.emplace("temperature", std::to_string(m_LastTemperature));
        output.emplace("relative_humidity", std::to_string(m_LastRelative_humidity));
        output.emplace("CO2", std::to_string(m_LastCO2));
    } else {
        output.emplace("temperature", "NaN");
        output.emplace("relative_humidity", "NaN");
        output.emplace("CO2", "NaN");
    }
    return output;
}

CSensorStatus CScdSensor::m_InitCallback() {
    if (!Wire.begin(47, 48)) {
        return CSensorStatus::Error("TwoWire failed to init!");
    }

    if (scd30.begin(SCD30_I2CADDR_DEFAULT, &Wire, 0)) {
        auto& calibration = CConfig::getInstance()["calibration"];

        if (calibration.valid()) {
            auto& scdSensorCalibration = calibration["scd"];

            if (scdSensorCalibration.valid()) {
                scd30.setTemperatureOffset(scdSensorCalibration.get<int>("temperatureOffset"));
            }
        }

        scd30.selfCalibrationEnabled(false);
        scd30.setMeasurementInterval(2);
        return CSensorStatus::Ok();
    } else {
        return CSensorStatus::Error("SCD30 failed to init!");
    }
}
