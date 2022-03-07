#include "app.hpp"
#include <stdexcept>
#include <iostream>

void App::init() {
    std::printf("app.init()\n");
}

void App::loop() {
    std::printf("app.loop()\n");
    throw std::runtime_error("If you see this, everything works!\n");
}

void App::start() {
    try {
        this->init();
        while (true) this->loop();
    }
    catch (const std::runtime_error &e) {
        std::printf(
            "Ah shucks!\n"
            "FATAL unhandled runtime exception occured!\n"
            "Famous lasts words:\n"
            "%s",
            e.what()
        );
    }
}