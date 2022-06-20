#pragma once

#include "esp_netif_types.h"
#include "wificredentials.hpp"

#include <functional>
#include <string>
#include <vector>

// forward declerations, they are the same as in their actual headers
typedef void* EventGroupHandle_t;
typedef int esp_err_t;

class CWifi {
public:
    /// wifi credentials
    WifiCredentials mCredentials;
    
    /// ip info
    esp_netif_ip_info_t mIp;

    /// Wifi callback type
    using WifiCb = std::function<void()>;
    
    /// @brief register a callback to be called when the wifi connects
    void mAttachOnConnect(WifiCb aFunc);
    
    /// @brief register a callback to be called when the wifi connection is lost 
    void mAttachOnDisconnect(WifiCb aFunc);

    /// @brief get instance of CWifi
    static CWifi& getInstance();

    /// @brief initialize WiFi with given credentials
    void mInitWifi(const WifiCredentials& aConfig);

    /// @brief get if we are currently connected
    bool mConnected();

    /// @brief disconnect the WiFi
    void mDisconnect();

private:
    /// eventgroup for the wifi
    EventGroupHandle_t m_EventGroup;

    /// netinterface pointer
    esp_netif_t* m_StaNetif = nullptr;

    CWifi();
    
    /// @brief event handler method to handle the wifi / ip events
    static void m_EventHandler(void* apArg, esp_event_base_t aBase, int32_t aId, void* apData);

    /// connection callbacks
    std::vector<WifiCb> m_ConnectCbs;
    /// disconnection callbacks
    std::vector<WifiCb> m_DisconnectCbs;
};
