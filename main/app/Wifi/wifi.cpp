#include "wifi.hpp"
#include <string.h>
#include <stdexcept>
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"

#include <logscope.hpp>

static CLogScope logger{"wifi"};
const int CONNECTED_BIT = BIT0;

// FIXME: use logger eventually
#define WIFI_THROW_ON_ERROR(method)                         \
    if ((error = method) != ESP_OK) {                       \
        throw std::runtime_error(esp_err_to_name(error));   \
    }

CWifi::CWifi() {}

CWifi& CWifi::getInstance() {
    static auto instance = CWifi();
    return instance;
}

void CWifi::m_EventHandler(void* apArg, esp_event_base_t aBase, int32_t aId, void* apData) {
    auto& self = CWifi::getInstance();

    if (aBase == WIFI_EVENT) {
        switch (aId)
        {
        case WIFI_EVENT_STA_START:
            logger.mDebug("WIFI_EVENT_STA_START");
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_CONNECTED:
            logger.mInfo("WIFI_EVENT_STA_CONNECTED");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            logger.mInfo("WIFI_EVENT_STA_DISCONNECTED");
            for (auto& func : self.m_DisconnectCbs) func();

            esp_wifi_connect();
            xEventGroupClearBits(self.m_EventGroup, CONNECTED_BIT);
            memset(&self.mIp, 0, sizeof(decltype(self.mIp)));
            break;
        default:
            logger.mDebug("Unhandled wifi event id: %d", aId);
            break;
        }
    } else if (aBase == IP_EVENT) {
        switch (aId)
        {
        case IP_EVENT_STA_GOT_IP:
            logger.mDebug("IP_EVENT_STA_GOT_IP");

            xEventGroupSetBits(self.m_EventGroup, CONNECTED_BIT);
            esp_netif_get_ip_info(self.m_StaNetif, &self.mIp);

            for (auto& func : self.m_ConnectCbs) func();
            break;
        case IP_EVENT_STA_LOST_IP:
            logger.mDebug("IP_EVENT_STA_LOST_IP");
            break;
        default:
            logger.mDebug("Unhandled ip event id: %d", aId);
            break;
        }
    } else {
        printf("WTF\n");
        throw std::runtime_error("UNKNOWN WIFI BASE EVENT: " + std::string(aBase));
    }
}

void CWifi::mInitWifi(const WifiCredentials& aConfig) {
    logger.mInfo("Initializing WiFi");
    bool enterprise = !aConfig.eapUsername.empty();
    esp_err_t error;
    mCredentials = aConfig;

    WIFI_THROW_ON_ERROR(esp_netif_init());
    m_EventGroup = xEventGroupCreate();
    WIFI_THROW_ON_ERROR(esp_event_loop_create_default());
    m_StaNetif = esp_netif_create_default_wifi_sta();
    if (!m_StaNetif) throw std::runtime_error("WTF WIFI");

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    WIFI_THROW_ON_ERROR( esp_wifi_init(&cfg) );
    WIFI_THROW_ON_ERROR( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &CWifi::m_EventHandler, NULL) );
    WIFI_THROW_ON_ERROR( esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &CWifi::m_EventHandler, NULL) );
    WIFI_THROW_ON_ERROR( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {};
    memcpy(wifi_config.sta.ssid, mCredentials.ssid.c_str(), mCredentials.ssid.size());
    if (!enterprise) {
        memcpy(wifi_config.sta.password, mCredentials.password.c_str(), mCredentials.password.size());
    }
    wifi_config.sta.pmf_cfg.required = false;
    logger.mInfo("Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    WIFI_THROW_ON_ERROR( esp_wifi_set_mode(WIFI_MODE_STA) );
    WIFI_THROW_ON_ERROR( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    if (enterprise) {
        WIFI_THROW_ON_ERROR( esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)mCredentials.eapId.c_str(), mCredentials.eapId.size()) );

        WIFI_THROW_ON_ERROR( esp_wifi_sta_wpa2_ent_set_username((uint8_t *)mCredentials.eapUsername.c_str(), mCredentials.eapUsername.size()) );
        WIFI_THROW_ON_ERROR( esp_wifi_sta_wpa2_ent_set_password((uint8_t *)mCredentials.password.c_str(), mCredentials.password.size()) );
        WIFI_THROW_ON_ERROR( esp_wifi_sta_wpa2_ent_enable() );
    }
    WIFI_THROW_ON_ERROR( esp_wifi_start() );

    logger.mDebug("Initializing WiFi");

    uint8_t retry = 0;
    const uint8_t retryCnt = 15;
    while (mIp.ip.addr == 0 && ++retry <= retryCnt) {
        logger.mDebug("Waiting...  (%d/%d)", retry, retryCnt);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    if (retry == retryCnt) throw std::runtime_error("Could not connect to WiFi.");
}

bool CWifi::mConnected() {
    return mIp.ip.addr != 0;
}

void CWifi::mAttachOnConnect(WifiCb aFunc) {
    m_ConnectCbs.push_back(aFunc);
}

void CWifi::mAttachOnDisconnect(WifiCb aFunc) {
    m_DisconnectCbs.push_back(aFunc);
}

void CWifi::mDisconnect() {
    esp_wifi_disconnect();
}

void CWifi::mDeinit() {
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
}
