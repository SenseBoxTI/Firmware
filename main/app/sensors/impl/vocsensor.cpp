#include <vocsensor.hpp>
#include <scdsensor.hpp>
#include <Adafruit_CCS811.h>
#include <logscope.hpp>

#include <CConfig.hpp>

static CLogScope logger{"CCS811"};

Adafruit_CCS811 ccs = Adafruit_CCS811();

CVocSensor::CVocSensor(std::string aName, CScdSensor& arScdSensor)
:   CSensor(aName),
    mr_ScdSensor(arScdSensor)
{}

SensorOutput CVocSensor::m_MeasureCallback() {
    SensorOutput output;
    uint8_t error;

    ccs.setEnvironmentalData(mr_ScdSensor.get_RelativeHumidity(), mr_ScdSensor.get_Temperature());

    if ((error = ccs.readData()) == 0) {
        output.emplace("TVOC ppb", static_cast<float>(ccs.getTVOC()));
    } else {
        logger.mWarn("TVOC ppb read fail, error code: %d", error);
    }

    return output;
}

CSensorStatus CVocSensor::m_InitCallback() {
    m_MeasureInterval = CCS811_MEASURE_INTERVAL_US;

    if (ccs.begin()) {
        ccs.setDriveMode(CCS811_DRIVE_MODE_10SEC); //sample rate
        return CSensorStatus::Ok();
    } else {
        return CSensorStatus::Error("Cant connect VOC sensor I2C!");
    }
}
