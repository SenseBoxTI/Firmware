#include "mqtt.hpp"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "esp_ota_ops.h"
#include <sys/param.h>
#include <logscope.hpp>
#include <string>
#include <sstream>
#include <file.hpp>

#define MQTT_URL "mqtts://test.ddss-sensebox.nl"

static CLogScope logger{"mqtt"};

CMqtt::CMqtt(const char* acpClientId, const char* acpAssetId)
:   mcp_ClientId(acpClientId),  // we can probably hardcore this
    mcp_AssetId(acpAssetId),    // this should be unique, config file
    mb_Provisioned(false)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    std::stringstream subscribeTopic;

    subscribeTopic << "provisioning/" << mcp_AssetId << "/response";
    mcp_SubscribeTopic = subscribeTopic.str().c_str();

    CFile certFile("ca-cert.pem");
    m_Cert = certFile.mRead();
    const char* cert = m_Cert.c_str();

    const esp_mqtt_client_config_t mqtt_cfg = {
        .host = MQTT_URL, // get this from the config file
        .user_context = this,
        .cert_pem = cert // the certificate as const char*
    };

    m_Client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(m_Client, MQTT_EVENT_ANY, m_EventHandler, NULL);
    esp_mqtt_client_start(m_Client);
}

void CMqtt::mSendMeasurement(const char* acpAttribute, const char* acpValue) {
    if (!mb_Connected || !mb_Provisioned) return;

    std::stringstream topic;
    topic << mcp_Realm << '/' << mcp_ClientId << "/writeattributevalue/" << acpAttribute << '/' << mcp_AssetId;

    int msg_id = esp_mqtt_client_publish(m_Client, topic.str().c_str(), acpValue, sizeof(acpValue), 0, 0);
    logger.mInfo("binary sent with msg_id=%d", msg_id);
}

void CMqtt::m_SendCustom(const char* acpTopic, const char* acpMsg) {
    int msg_id = esp_mqtt_client_publish(m_Client, acpTopic, acpMsg, sizeof(acpMsg), 0, 0);
    logger.mInfo("binary sent with msg_id=%d", msg_id);
}

void CMqtt::m_Subscribe(const char* acpTopic) {
    esp_mqtt_client_subscribe(m_Client, acpTopic, 0);
}

void CMqtt::m_Unsubscribe(const char* acpTopic) {
    esp_mqtt_client_unsubscribe(m_Client, acpTopic);
}

void CMqtt::m_EventHandler(void* apArgs, esp_event_base_t aBase, int32_t aId, void* apData) {
    logger.mDebug("Event dispatched from event loop base=%s, event_id=%d", aBase, aId);
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(apData);
    esp_mqtt_client_handle_t client = event->client;

    CMqtt& self = *static_cast<CMqtt*>(event->user_context);

    int msg_id;
    switch (static_cast<esp_mqtt_event_id_t>(aId)) {
    case MQTT_EVENT_CONNECTED:
        self.mb_Connected = true;

        if (!self.mb_Provisioned) self.m_RequestProvision();
        break;
    case MQTT_EVENT_DISCONNECTED:
        self.mb_Connected = false;

        logger.mWarn("MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        logger.mDebug("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        logger.mDebug("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        logger.mDebug("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA: {
        if (strcmp(self.mcp_SubscribeTopic, event->topic)) {
            self.m_OnProvisionResponse(event->data, event->data_len);
        }
    }
    case MQTT_EVENT_ERROR:
        logger.mError("MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            logger.mError("Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            logger.mError("Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            logger.mError("Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                          strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            logger.mError("Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            logger.mError("Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        logger.mInfo("Other event id:%d", event->event_id);
        break;
    }
}

void CMqtt::m_RequestProvision() {
    std::stringstream publishTopic, publishData;
    publishTopic << "provisioning/" << mcp_AssetId << "/request";

    publishData << "{\"type\":\"x509\",\"cert\":\"" << m_Cert << "\"}";
    auto publishDataStr = publishData.str();

    m_Subscribe(mcp_SubscribeTopic);

    m_SendCustom(publishTopic.str().c_str(), publishDataStr.c_str());
}

void CMqtt::m_OnProvisionResponse(const char* acpData, int aLen) {
    const cJSON *type = NULL;
    const cJSON *realm = NULL;
    const cJSON *error = NULL;

    m_Unsubscribe(mcp_SubscribeTopic);

    int status = 0;
    cJSON* data = cJSON_ParseWithLength(acpData, aLen);
    if (data == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) logger.mWarn("Error before: %s\n", error_ptr);
        m_JsonError(data, "Error parsing provisioning JSON");
    }

    type = cJSON_GetObjectItemCaseSensitive(data, "type");
    if (cJSON_IsString(type) && (type->valuestring != NULL)) {
        if (strcmp(type->valuestring, "error")) {
            error = cJSON_GetObjectItemCaseSensitive(data, "error");
            if (cJSON_IsString(error) && (error->valuestring != NULL)) {
                std::string errorMsg("Could not provision device: ");
                errorMsg.append(error->valuestring);

                m_JsonError(data, errorMsg.c_str());
            } else {
                m_JsonError(data, "Could not parse provisioning response.error");
            }
        }

        realm = cJSON_GetObjectItemCaseSensitive(data, "realm");

        if (cJSON_IsString(realm) && (realm->valuestring != NULL)) {
            mcp_Realm = realm->valuestring;
        } else {
            m_JsonError(data, "Could not parse provisioning response.realm");
        }
    } else {
        m_JsonError(data, "Could not parse provisioning response.type");
    }

    cJSON_Delete(data);

    mb_Provisioned = true;
}

void CMqtt::m_JsonError(cJSON* aJsonObject, const char* aError) {
    cJSON_Delete(aJsonObject);
    throw std::runtime_error(aError);
}
