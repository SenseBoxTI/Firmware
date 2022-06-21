#include "wifi.hpp"
#include <string.h>
#include <stdexcept>
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include <freertos/task.h>

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

    try {
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
            case WIFI_EVENT_STA_DISCONNECTED: {
                logger.mInfo("WIFI_EVENT_STA_DISCONNECTED");
                for (auto& func : self.m_DisconnectCbs) func();

                xEventGroupClearBits(self.m_EventGroup, CONNECTED_BIT);
                memset(&self.mIp, 0, sizeof(decltype(self.mIp)));

                // create task to move of the event task, low priority so it does not block other wifi events
                xTaskHandle handle = NULL;
                xTaskCreatePinnedToCore(&m_Reconnect, "wifiReconnect", CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT, &self, 2, &handle, 1);
                break;
            }
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
    catch (const std::runtime_error& e) {
        logger.mError("Error during event: %s.", e.what());
    }
}

void CWifi::mInitWifi(const WifiCredentials& aConfig) {
    logger.mInfo("Initializing WiFi");
    bool enterprise = !aConfig.eapUsername.empty();
    esp_err_t error;
    mCredentials = aConfig;

    if (mCredentials.ssid.size() > 32) throw std::runtime_error("SSID is longer than 32 characters");
    if (enterprise && mCredentials.password.size() > 64) throw std::runtime_error("Password is longer than 64 characters");

    if (m_StaNetif == nullptr) {
        WIFI_THROW_ON_ERROR(esp_netif_init());
        m_EventGroup = xEventGroupCreate();
        WIFI_THROW_ON_ERROR(esp_event_loop_create_default());
        m_StaNetif = esp_netif_create_default_wifi_sta();
        if (!m_StaNetif) throw std::runtime_error("WTF WIFI");
    }

    WIFI_THROW_ON_ERROR( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &CWifi::m_EventHandler, NULL) );
    WIFI_THROW_ON_ERROR( esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &CWifi::m_EventHandler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    WIFI_THROW_ON_ERROR( esp_wifi_init(&cfg) );
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
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &CWifi::m_EventHandler);
    esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, &CWifi::m_EventHandler);

    logger.mDebug("wifi_init_sta stop sta");
    ESP_ERROR_CHECK(esp_wifi_stop());

    logger.mDebug("wifi_init_sta deinit sta");
    ESP_ERROR_CHECK(esp_wifi_deinit());

    for (auto& func : m_DisconnectCbs) func();

    xEventGroupClearBits(m_EventGroup, CONNECTED_BIT);
    memset(&mIp, 0, sizeof(decltype(mIp)));

    m_ConnectCbs.clear();
    m_DisconnectCbs.clear();
    logger.mDebug("wifi succesfully deinited");
}

void CWifi::m_Reconnect(void* aSelf) {
    CWifi& self = *static_cast<CWifi*>(aSelf);

    logger.mInfo("Reconnecting to network: %s.", self.mCredentials.ssid.c_str());
    esp_wifi_connect();

    vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);

    while (!self.mConnected()) {
        logger.mInfo("Reconnecting to network: %s.", self.mCredentials.ssid.c_str());
        esp_wifi_disconnect();
        esp_wifi_connect();
        vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}
