#include "vocsensor.hpp"
#include "scdsensor.hpp"
#include <Adafruit_CCS811.h>
#include <logscope.hpp>
#include <config.hpp>

static CLogScope logger{"vocsensor"};

Adafruit_CCS811 ccs = Adafruit_CCS811();

CVocSensor::CVocSensor(std::string aName, CScdSensor& arScdSensor)
:   CSensor(aName),
    mr_ScdSensor(arScdSensor) {

}

SensorOutput CVocSensor::m_MeasureCallback() {
    SensorOutput output;
    uint8_t error;
    ccs.setEnvironmentalData(mr_ScdSensor.get_LastRelative_humidity(), mr_ScdSensor.get_LastTemperature());
    if ((error = ccs.readData()) == 0) {
        output.emplace("TVOC ppb", std::to_string(ccs.getTVOC() * m_factor));
    } else {
        logger.mError("TVOC ppb read fail | error code: %d", error);
    }

    return output;
}

CSensorStatus CVocSensor::m_InitCallback() {
    if(ccs.begin()) {
        ccs.setDriveMode(CCS811_DRIVE_MODE_10SEC);//sample rate

        auto& calibration = CConfig::getInstance()["calibration"];
        if (calibration.valid()) {
            auto vocSensorCalibration = calibration["voc"];
            if (vocSensorCalibration.valid()) {
                m_factor = vocSensorCalibration.get<double>("factor");
            }
        }
        return CSensorStatus::Ok();
    } else {
        return CSensorStatus::Error("Cant connect VOC sensor I2C!");
    }
}
