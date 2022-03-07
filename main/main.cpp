#include <Arduino.h>
#include "app/app.hpp"

extern "C" void app_main();

void app_main() {
    initArduino();

    auto app = App();
    app.start();

    while (true); //hang
}