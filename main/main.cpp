#include <Arduino.h>
#include <esp_task.h>
#include <app.hpp>

extern "C" void app_main();

void app_main() {
    initArduino();

    auto app = App();
    app.start();
}