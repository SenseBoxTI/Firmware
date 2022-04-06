#include "i2ctest.hpp"

#include <stdexcept>
#include <Adafruit_SCD30.h>

Adafruit_SCD30 scd30; 

SensorOutput CI2cTest::m_MeasureCallback() {
    SensorOutput output;

    if (!scd30.read()) {
        throw std::runtime_error("Failed to read SCD30");
    };
    
    static char buffer[24] = { 0 };
    std::snprintf(&buffer[0b00000], 8, "%.2f", scd30.CO2);
    std::snprintf(&buffer[0b01000], 8, "%.2f", scd30.relative_humidity);
    std::snprintf(&buffer[0b10000], 8, "%.2f", scd30.temperature);
    
    output.insert({"CO2 ppm", &buffer[0]});
    output.insert({"Relative Humid %", &buffer[8]});
    output.insert({"Temperature deg C", &buffer[16]});

    return output;
}

CSensorStatus CI2cTest::m_InitCallback() {
    if (!scd30.begin()) {
        return CSensorStatus::Error("SC30 failed to init!");
    }
    return CSensorStatus::Ok();
}