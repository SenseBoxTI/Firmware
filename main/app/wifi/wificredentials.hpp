#pragma once

#include <string>
// The credentials required to be filed when connecting to the Wifi
struct WifiCredentials {
    std::string ssid;
    std::string eapId;
    std::string eapUsername;
    std::string password;
};
