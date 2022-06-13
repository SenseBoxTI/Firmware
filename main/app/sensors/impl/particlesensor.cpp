#include "particlesensor.hpp"
#include <Adafruit_PM25AQI.h>

Adafruit_PM25AQI pmsa = Adafruit_PM25AQI();

SensorOutput CParticleSensor::m_MeasureCallback(){
    SensorOutput output;
    PM25_AQI_Data data;
    if(pmsa.read(&data)){
        // output.emplace("PM 1.0", std::to_string(data.pm10_standard));
        output.emplace("PM 2.5", std::to_string(data.pm25_standard));

    }
    else{
        output.emplace("PM 2.5"," Read fail");
    }
    return output;
}

CSensorStatus CParticleSensor::m_InitCallback(){
    if(pmsa.begin_I2C()){
        return CSensorStatus::Ok();
        
    }
    else{
        return CSensorStatus::Error("Cant connect particle sensor I2C!");
    }
}