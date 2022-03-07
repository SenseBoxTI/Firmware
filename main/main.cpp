#include <Arduino.h>
#include <esp_task.h>
#include <app.hpp>

extern "C" void app_main();

void app_main() {
    initArduino();

    auto app = App();
    app.start();

    // hang, update watchdog once in a while to keep RTOS happy
    while (true) vTaskDelay(1000 / portTICK_PERIOD_MS);
}