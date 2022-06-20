#include <Arduino.h>
#include <esp_task.h>
#include <app.hpp>

extern "C" void app_main();

/**
 * This is the main application entry point
 * It is extern "C" because the compiler requires it to be.
 * C++ can later be used throughout the project, however.
 */
void app_main() {
    initArduino();

    auto app = App();
    app.start();

    while (!App::stopped) vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("App has stopped\n");

    while (true) vTaskDelay(60 * 1000 / portTICK_PERIOD_MS);
}