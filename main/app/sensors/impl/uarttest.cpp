#include "uarttest.hpp"

#include <stdexcept>
#include <Adafruit_PM25AQI.h>

Adafruit_PM25AQI pm25aqi;
#define RX_PIN 31
#define TX_PIN 32

SensorOutput CUartTest::m_MeasureCallback() {
    SensorOutput output;

    PM25_AQI_Data data;
    if (!pm25aqi.read(&data)) {
        throw std::runtime_error("Failed to read SCD30");
    };

    static char buffer[8] = { 0 };
    std::snprintf(&buffer[0], 8, "%d", data.particles_25um);

    output.insert({"PM25", &buffer[0]});    
    return output;
}

CSensorStatus CUartTest::m_InitCallback() {

    Serial2.begin(9600, SERIAL_8N1,  RX_PIN, TX_PIN);

    if (!pm25aqi.begin_UART(&Serial2)) {
        return CSensorStatus::Error("Failed to init.");
    }

    return CSensorStatus::Ok();
}