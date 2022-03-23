#pragma once

#include <string>
struct WifiCredentials {
    std::string ssid;
    std::string eapId;
    std::string eapUsername;
    std::string password;
};