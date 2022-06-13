#include "vocsensor.hpp"
#include "scdsensor.hpp"
#include <Adafruit_CCS811.h>


Adafruit_CCS811 ccs = Adafruit_CCS811();

CVocSensor::CVocSensor(std::string aName, CScdSensor& arScdSensor)
:   CSensor(aName),
    mr_ScdSensor(arScdSensor){

}

SensorOutput CVocSensor::m_MeasureCallback(){
    SensorOutput output;
    uint8_t error;
    ccs.setEnvironmentalData(mr_ScdSensor.get_LastRelative_humidity(), mr_ScdSensor.get_LastTemperature());
    if ((error = ccs.readData()) == 0){
        // output.emplace("CO2", std::to_string(ccs.geteCO2()));
        output.emplace("TVOC ppb", std::to_string(ccs.getTVOC()));
    }
    else{
        // output.emplace("VOC sensor read (CCS811) error","");
        output.emplace("TVOC ppb read fail | error code", std::to_string(error));
    }
    return output;
}

CSensorStatus CVocSensor::m_InitCallback(){
    if(ccs.begin()){
        ccs.setDriveMode(CCS811_DRIVE_MODE_10SEC);//sample rate
        return CSensorStatus::Ok();
    }
    else{
        return CSensorStatus::Error("Cant connect VOC sensor I2C!");
    }
}
