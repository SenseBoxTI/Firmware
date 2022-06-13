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
#include <file.hpp>
#include <config.hpp>
#include <CConfig.hpp>

static CLogScope logger{"mqtt"};

CMqtt::CMqtt()
:   mb_Provisioned(false),
    mb_Connected(false),
    mb_SendAttributes(false)
{}

CMqtt& CMqtt::getInstance() {
    static CMqtt instance = {};
    return instance;
}

void CMqtt::mInit(const std::string& acrDeviceId, const std::string& acrAccessToken) {
    mcp_DeviceId = acrDeviceId;
    mcp_AccessToken = acrAccessToken;

    ESP_ERROR_CHECK(nvs_flash_init());

    mcp_SubscribeTopic = "/provision/response";

    const esp_mqtt_client_config_t mqtt_cfg = m_GetClientConfig("provision");

    m_Client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(m_Client, MQTT_EVENT_ANY, m_EventHandler, NULL);
    esp_mqtt_client_start(m_Client);
}

void CMqtt::mSendMeasurements(Measurements& arValues) {
    if (!mb_Connected || !mb_Provisioned) return;

    std::string topic("v1/devices/me/telemetry");

    if (!mb_SendAttributes) {
        topic = "v1/devices/me/attributes";
        mb_SendAttributes = true;
    }

    cJSON* obj = cJSON_CreateObject(); 
    if (obj == NULL) throw std::runtime_error("Could not create measurements object");

    for (auto& el : arValues) {
        for (auto& p : el.second) {
            if (cJSON_GetObjectItemCaseSensitive(obj, p.first.c_str()) == NULL) {
                cJSON_AddNumberToObject(obj, p.first.c_str(), static_cast<double>(p.second));
            }

            // else get average value?
        }
    }

    const char* publishData = cJSON_Print(obj);
    if (publishData == NULL) m_JsonError(obj, "Could not print json object");

    cJSON_Delete(obj);

    m_SendCustom(topic.c_str(), publishData);
}

void CMqtt::m_SendCustom(const char* acpTopic, const char* acpMsg) {
    int msg_id = esp_mqtt_client_publish(m_Client, acpTopic, acpMsg, strlen(acpMsg), 0, 0);
    logger.mInfo("binary sent with msg_id=%d", msg_id);
}

void CMqtt::m_Subscribe(const char* acpTopic) {
    esp_mqtt_client_subscribe(m_Client, acpTopic, 0);
}

void CMqtt::m_Unsubscribe(const char* acpTopic) {
    esp_mqtt_client_unsubscribe(m_Client, acpTopic);
}

void CMqtt::m_EventHandler(void* apArgs, esp_event_base_t aBase, int32_t aId, void* apData) {
    logger.mDebug("Event dispatched from event loop base=%s, event_id=%d", static_cast<const char*>(aBase), aId);
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(apData);

    CMqtt& self = *static_cast<CMqtt*>(event->user_context);

    try {
    switch (static_cast<esp_mqtt_event_id_t>(aId)) {
    case MQTT_EVENT_CONNECTED:
        self.mb_Connected = true;

        logger.mDebug("MQTT_EVENT_CONNECTED");

        if (!self.mb_Provisioned) self.m_RequestProvision();
        break;
    case MQTT_EVENT_DISCONNECTED: {
        self.mb_Connected = false;
        logger.mWarn("MQTT_EVENT_DISCONNECTED");

        if (self.mb_Provisioned) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            self.m_Reconnect();
        }

        break;
    }
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
        logger.mDebug("MQTT_EVENT_DATA");

        if (strcmp(self.mcp_SubscribeTopic, event->topic) == 0) {
            self.m_OnProvisionResponse(event->data, event->data_len);
        }
        break;
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
    } catch (const std::runtime_error& err) {
        logger.mError(err.what());
    }
}

void CMqtt::m_RequestProvision() {
    const char* publishTopic = "/provision/request";
    const char* publishData;

    cJSON* obj = cJSON_CreateObject();
    if (obj == NULL) throw std::runtime_error("Could not create provisioning object");

    if (cJSON_AddStringToObject(obj, "deviceName", mcp_DeviceId.c_str()) == NULL) {
        m_JsonError(obj, "Could not create deviceName for provisioning object");
    }
    if (cJSON_AddStringToObject(obj, "provisionDeviceKey", PROVISION_KEY) == NULL) {
        m_JsonError(obj, "Could not create provisionDeviceKey for provisioning object");
    }
    if (cJSON_AddStringToObject(obj, "provisionDeviceSecret", PROVISION_SECRET) == NULL) {
        m_JsonError(obj, "Could not create provisionDeviceSecret for provisioning object");
    }
    if (cJSON_AddStringToObject(obj, "credentialsType", "ACCESS_TOKEN") == NULL) {
        m_JsonError(obj, "Could not create credentialsType for provisioning object");
    }
    if (cJSON_AddStringToObject(obj, "token", mcp_AccessToken.c_str()) == NULL) {
        m_JsonError(obj, "Could not create token for provisioning object");
    }

    publishData = cJSON_Print(obj);
    if (publishData == NULL) m_JsonError(obj, "Could not print json object");

    cJSON_Delete(obj);

    m_Subscribe(mcp_SubscribeTopic);

    m_SendCustom(publishTopic, publishData);
}

void CMqtt::m_OnProvisionResponse(const char* acpData, int aLen) {
    m_Unsubscribe(mcp_SubscribeTopic);

    cJSON* data = cJSON_ParseWithLength(acpData, aLen);
    if (data == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) logger.mWarn("Error before: %s\n", error_ptr);
        m_JsonError(data, "Error parsing provisioning JSON");
    }

    cJSON* status = cJSON_GetObjectItemCaseSensitive(data, "status");
    if (cJSON_IsString(status) && (status->valuestring != NULL)) {
        if (strcmp(status->valuestring, "SUCCESS") == 0) {
            logger.mInfo("Provisioned new device");
            } else {
            cJSON* errorMsg = cJSON_GetObjectItemCaseSensitive(data, "errorMsg");
            if (cJSON_IsString(errorMsg) && (errorMsg->valuestring != NULL)) {
                if (strcmp(errorMsg->valuestring, "Failed to provision device!") == 0) {
                    logger.mInfo("Device was already provisioned");
                } else {
                    std::string error("Could not provision device: ");
                    error.append(errorMsg->valuestring);

                    throw std::runtime_error(error);
            }
        } else {
                m_JsonError(data, "Could not parse provision errorMsg");
            }
        }
    } else {
        m_JsonError(data, "Could not parse provision status");
    }

    cJSON_Delete(data);

    esp_mqtt_client_disconnect(m_Client);

    mb_Provisioned = true;
}

esp_mqtt_client_config_t CMqtt::m_GetClientConfig(const char* aUsername) {
    esp_mqtt_client_config_t config = {
        .uri = MQTT_URL,
        .client_id = NULL,
        .username = aUsername,
        .disable_auto_reconnect = true, // done in the event handler
        .user_context = this,
        .cert_pem = MQTT_TLS_CERT,
        .skip_cert_common_name_check = true,
    };
    return config;
}

void CMqtt::m_Reconnect() {
    const esp_mqtt_client_config_t mqtt_cfg = m_GetClientConfig(mcp_AccessToken.c_str());
    m_Client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(m_Client, MQTT_EVENT_ANY, m_EventHandler, NULL);
    esp_mqtt_client_start(m_Client);
}

void CMqtt::m_JsonError(cJSON* aJsonObject, const char* aError) {
    cJSON_Delete(aJsonObject);
    throw std::runtime_error(aError);
}
