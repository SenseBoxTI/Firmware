#pragma once

#include "esp_err.h"
#include "esp_netif_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include <string>

struct SPeapCredentials {
    std::string m_ssid;
    std::string m_eapId;
    std::string m_eapUsername;
    std::string m_eapPassword;
};

struct SWpaCredentials {
    std::string m_ssid;
    std::string m_password;
};

class CWifi {
    esp_netif_ip_info_t m_ip;
    EventGroupHandle_t m_wifi_event_group;
    bool m_connected = false;
    esp_err_t m_error = ESP_OK;
    esp_netif_t *m_sta_netif = nullptr;

        CWifi();
        static void m_EventHandler(void* apArg, esp_event_base_t aBase, int32_t ald, void* apData);

    public:
        /// @brief get instance of CWifi
        static CWifi& getInstance();

        /// @brief initialize WiFi with target PEAP network
        esp_err_t mInitWifi(SPeapCredentials aConfig);

        /// @brief initialize WiFi with target WPA2 network
        esp_err_t mInitWifi(SWpaCredentials aConfig);
        
        /// @brief get if we are currently connected 
        bool mConnected();
};