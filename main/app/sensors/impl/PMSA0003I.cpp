#include <PMSA0003I.hpp>
#include <Adafruit_PM25AQI.h>
#include <logscope.hpp>
#include <config.hpp>
#include <CConfig.hpp>

static CLogScope logger{"particlesensor"};

Adafruit_PM25AQI pmsa = Adafruit_PM25AQI();

SensorOutput CPmsa0003I::m_MeasureCallback() {
    SensorOutput output;
    PM25_AQI_Data data;

    if (pmsa.read(&data)) {
        output.emplace("PM 2.5", static_cast<float>(data.pm25_standard) * m_factor);
    } else {
        logger.mWarn("Read fail");
    }

    return output;
}

CSensorStatus CPmsa0003I::m_InitCallback() {
    m_MeasureInterval = PMSA003I_MEASURE_INTERVAL_US;

    auto& calibration = CConfig::getInstance()["calibration"];

    if (calibration.valid()) {
        auto& particleSensorCalibration = calibration["particles"];

        if (particleSensorCalibration.valid()) {
            m_factor = particleSensorCalibration.get<double>("factor");
        }
    }

    if (pmsa.begin_I2C()) {
        return CSensorStatus::Ok();
    } else {
        return CSensorStatus::Error("Cant connect particle sensor I2C!");
    }
}
