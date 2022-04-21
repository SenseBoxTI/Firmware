#include "uarttest.hpp"

#include <stdexcept>
#include <Adafruit_PM25AQI.h>
#include <driver/gpio.h>

Adafruit_PM25AQI pm25aqi;
#define RESET_GPIO 27
#define RX_GPIO 28
#define TX_GPIO 29
#define SET_GPIO 30

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

    gpio_set_level(static_cast<gpio_num_t>(RESET_GPIO), 1);
    gpio_set_level(static_cast<gpio_num_t>(SET_GPIO), 1);

    Serial2.begin(9600, SERIAL_8N1,  RX_GPIO, TX_GPIO);

    if (!pm25aqi.begin_UART(&Serial2)) {
        return CSensorStatus::Error("Failed to init.");
    }

    return CSensorStatus::Ok();
}