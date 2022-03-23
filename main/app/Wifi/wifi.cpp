#include "wifi.hpp"
#include <string.h>
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "freertos/task.h"

const int CONNECTED_BIT = BIT0;

// FIXME: use logger eventually                         
#define WIFI_ERR_CHECK(meth)                                    \
    if ((error = meth) != ESP_OK) {                           \
        std::printf("Got error: %s", esp_err_to_name(error)); \
        return error;                                         \
    }

CWifi::CWifi() {}

CWifi& CWifi::getInstance() {
    static auto instance = CWifi();
    return instance;
}

void CWifi::m_EventHandler(void* apArg, esp_event_base_t aBase, int32_t aId, void* apData) {
    if (aBase == WIFI_EVENT && aId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (aBase == WIFI_EVENT && aId == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        auto& wifi = CWifi::getInstance();
        xEventGroupClearBits(wifi.m_EventGroup, CONNECTED_BIT);
        memset(&wifi.mIp, 0, sizeof(decltype(wifi.mIp)));
    } else if (aBase == IP_EVENT && aId == IP_EVENT_STA_GOT_IP) {
        auto& wifi = CWifi::getInstance();
        xEventGroupSetBits(wifi.m_EventGroup, CONNECTED_BIT);
        esp_netif_get_ip_info(wifi.m_StaNetif, &wifi.mIp);
    }
}

esp_err_t CWifi::mInitWifi(const WifiCredentials& aConfig) {
    // TODO: more verbose logging?
    bool enterprise = !aConfig.eapUsername.empty();
    esp_err_t error;
    mCredentials = aConfig;
    
    WIFI_ERR_CHECK(esp_netif_init());
    m_EventGroup = xEventGroupCreate();
    WIFI_ERR_CHECK(esp_event_loop_create_default());
    m_StaNetif = esp_netif_create_default_wifi_sta();
    assert(m_StaNetif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    WIFI_ERR_CHECK( esp_wifi_init(&cfg) );
    WIFI_ERR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &CWifi::m_EventHandler, NULL) );
    WIFI_ERR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &CWifi::m_EventHandler, NULL) );
    WIFI_ERR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {};
    memcpy(wifi_config.sta.ssid, mCredentials.ssid.c_str(), mCredentials.ssid.size());
    if (!enterprise) {
        memcpy(wifi_config.sta.ssid, mCredentials.password.c_str(), mCredentials.password.size());
    }
    wifi_config.sta.pmf_cfg.required = false;
    // FIXME: use logger eventually
    std::printf("Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    WIFI_ERR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    WIFI_ERR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    if (enterprise) {
        WIFI_ERR_CHECK( esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)mCredentials.eapId.c_str(), mCredentials.eapId.size()) );
    
        WIFI_ERR_CHECK( esp_wifi_sta_wpa2_ent_set_username((uint8_t *)mCredentials.eapUsername.c_str(), mCredentials.eapUsername.size()) );
        WIFI_ERR_CHECK( esp_wifi_sta_wpa2_ent_set_password((uint8_t *)mCredentials.password.c_str(), mCredentials.password.size()) );
        WIFI_ERR_CHECK( esp_wifi_sta_wpa2_ent_enable() );
    }
    WIFI_ERR_CHECK( esp_wifi_start() );
    return ESP_OK;
}

bool CWifi::mConnected() {
    return mIp.ip.addr != 0;
}
