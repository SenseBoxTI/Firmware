#pragma once

#include "esp_err.h"
#include "esp_netif_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "wificredentials.hpp"

#include <string>

class CWifi {
        EventGroupHandle_t m_EventGroup;
        esp_netif_t* m_StaNetif = nullptr;
        CWifi();
        static void m_EventHandler(void* apArg, esp_event_base_t aBase, int32_t aId, void* apData);

    public:
        WifiCredentials mCredentials;
        esp_netif_ip_info_t mIp;

        /// @brief get instance of CWifi
        static CWifi& getInstance();

        /// @brief initialize WiFi with given credentials
        esp_err_t mInitWifi(const WifiCredentials& aConfig);

        /// @brief get if we are currently connected
        bool mConnected();
};