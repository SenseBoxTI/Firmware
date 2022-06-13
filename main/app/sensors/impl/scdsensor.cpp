#include "scdsensor.hpp"
#include <Adafruit_SCD30.h>

#include <CConfig.hpp>

Adafruit_SCD30 scd30 = Adafruit_SCD30();

SensorOutput CScdSensor::m_MeasureCallback() {
    SensorOutput output;

    if (scd30.dataReady() && scd30.read()) {
        output.emplace("temperature", scd30.temperature);
        output.emplace("relative_humidity", scd30.relative_humidity);
        output.emplace("CO2", scd30.CO2);

        m_LastTemperature = scd30.temperature;
        m_LastRelative_humidity = scd30.relative_humidity;
    }

    return output;
}

CSensorStatus CScdSensor::m_InitCallback() {
    m_MeasureInterval = SCD30_MEASURE_INTERVAL_US;

    if (!Wire.begin(47, 48)) {
        return CSensorStatus::Error("TwoWire failed to init!");
    }

    if (scd30.begin(SCD30_I2CADDR_DEFAULT, &Wire, 0)) {
        scd30.selfCalibrationEnabled(false);
        scd30.setTemperatureOffset(0);
        scd30.setMeasurementInterval(2);
        return CSensorStatus::Ok();
    } else {
        return CSensorStatus::Error("SCD30 failed to init!");
    }
}
