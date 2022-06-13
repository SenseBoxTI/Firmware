#pragma once

#include "esp_netif_types.h"
#include "wificredentials.hpp"

#include <functional>
#include <string>
#include <vector>

typedef void* EventGroupHandle_t;
typedef int esp_err_t;
class CWifi {
public:
    WifiCredentials mCredentials;
    esp_netif_ip_info_t mIp;

    using WifiCb = std::function<void()>;
    void mAttachOnConnect(WifiCb aFunc);
    void mAttachOnDisconnect(WifiCb aFunc);

    /// @brief get instance of CWifi
    static CWifi& getInstance();

    /// @brief initialize WiFi with given credentials
    void mInitWifi(const WifiCredentials& aConfig);

    /// @brief get if we are currently connected
    bool mConnected();

private:
    EventGroupHandle_t m_EventGroup;
    esp_netif_t* m_StaNetif = nullptr;
    CWifi();
    static void m_EventHandler(void* apArg, esp_event_base_t aBase, int32_t aId, void* apData);

    std::vector<WifiCb> m_ConnectCbs;
    std::vector<WifiCb> m_DisconnectCbs;
};
