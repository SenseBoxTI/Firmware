#include <Arduino.h>
#include <esp_task.h>
#include <app.hpp>

extern "C" void app_main();

void app_main() {
    initArduino();

    auto& app = App::getInstance();
    app.start();

    while (App::status != Stopped) vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("App has stopped\n");

    while (true) vTaskDelay(60 * 1000 / portTICK_PERIOD_MS);
}
