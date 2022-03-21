#include "wifi.hpp"
#include <string.h>
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "freertos/task.h"

#define WIFI_ERR_CHECK(meth)                                    \
    if ((m_error = meth) != ESP_OK) {                           \
        std::printf("Got error: %s", esp_err_to_name(m_error));   \
        return m_error;                                         \
    }

CWifi::CWifi() {}

CWifi& CWifi::getInstance() {
    static auto instance = CWifi();
    return instance;
}

void CWifi::m_EventHandler(void* apArg, esp_event_base_t aBase, int32_t ald, void* apData) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        CWifi::getInstance().m_connected = false;
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        CWifi::getInstance().m_connected = true;
    }
}

esp_err_t CWifi::mInitWifi(SPeapCredentials aConfig) {
    WIFI_ERR_CHECK(esp_netif_init());
    m_wifi_event_group = xEventGroupCreate();
    WIFI_ERR_CHECK(esp_event_loop_create_default());
    m_sta_netif = esp_netif_create_default_wifi_sta();
    assert(m_sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    WIFI_ERR_CHECK( esp_wifi_init(&cfg) );
    WIFI_ERR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &CWifi::m_EventHandler, NULL) );
    WIFI_ERR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &CWifi::m_EventHandler, NULL) );
    WIFI_ERR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {};
    memcpy(wifi_config.sta.ssid, aConfig.m_ssid.c_str(), aConfig.m_ssid.size());
    wifi_config.sta.pmf_cfg.required = false;
    std::printf("Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    WIFI_ERR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    WIFI_ERR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    WIFI_ERR_CHECK( esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)aConfig.m_eapId.c_str(), aConfig.m_eapId.size()) );
    
    WIFI_ERR_CHECK( esp_wifi_sta_wpa2_ent_set_username((uint8_t *)aConfig.m_eapUsername.c_str(), aConfig.m_eapUsername.size()) );
    WIFI_ERR_CHECK( esp_wifi_sta_wpa2_ent_set_password((uint8_t *)aConfig.m_eapPassword.c_str(), aConfig.m_eapPassword.size()) );

    WIFI_ERR_CHECK( esp_wifi_sta_wpa2_ent_enable() );
    WIFI_ERR_CHECK( esp_wifi_start() );
    return ESP_OK;
}

esp_err_t CWifi::mInitWifi(SWpaCredentials aConfig) {
    WIFI_ERR_CHECK(esp_netif_init());
    m_wifi_event_group = xEventGroupCreate();
    WIFI_ERR_CHECK(esp_event_loop_create_default());
    m_sta_netif = esp_netif_create_default_wifi_sta();
    assert(m_sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    WIFI_ERR_CHECK( esp_wifi_init(&cfg) );
    WIFI_ERR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &CWifi::m_EventHandler, NULL) );
    WIFI_ERR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &CWifi::m_EventHandler, NULL) );
    WIFI_ERR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {};
    memcpy(wifi_config.sta.ssid, aConfig.m_ssid.c_str(), aConfig.m_ssid.size());
    memcpy(wifi_config.sta.ssid, aConfig.m_password.c_str(), aConfig.m_password.size());
    wifi_config.sta.pmf_cfg.required = false;
    std::printf("Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    WIFI_ERR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    WIFI_ERR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

    WIFI_ERR_CHECK( esp_wifi_start() );
    return ESP_OK;
}

bool CWifi::mConnected() {
    return m_connected;
}
