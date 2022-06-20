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
#include <wifi.hpp>
#include <freertos/task.h>
#include <app.hpp>

static CLogScope logger{"mqtt"};

CMqtt::CMqtt()
:   m_Client(NULL),
    mb_Provisioned(false),
    mb_Connected(false),
    mb_SendAttributes(false),
    m_ReconnectCnt(0),
    m_DisconnectedRequestCnt(0)
{}

CMqtt& CMqtt::getInstance() {
    static CMqtt instance = {};
    return instance;
}

void CMqtt::mInit(const std::string& acrDeviceId, const std::string& acrAccessToken) {
    logger.mInfo("Initializing MQTT");

    auto& wifi = CWifi::getInstance();
    if (!wifi.mConnected()) throw std::runtime_error("Need a active internet connection to connect to the MQTT server.");

    mcp_DeviceId = acrDeviceId;
    mcp_AccessToken = acrAccessToken;
    mcp_SubscribeTopic = "/provision/response";

    ESP_ERROR_CHECK(nvs_flash_init());

    const esp_mqtt_client_config_t mqtt_cfg = m_GetClientConfig("provision");

    m_Client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(m_Client);

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(m_Client, MQTT_EVENT_ANY, m_EventHandler, NULL);

    logger.mDebug("Connecting to server");

    uint8_t retry = 0;
    const uint8_t retryCnt = 15;
    while (!mb_Connected && ++retry <= retryCnt) {
        logger.mDebug("Waiting...  (%d/%d)", retry, retryCnt);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    if (retry == retryCnt) throw std::runtime_error("Could not connect to MQTT server.");
}

/**
 * static wrapper for m_Reinit(), to be called as task from the event handler
 */
void CMqtt::m_Reinit(void* aSelf) {
    CMqtt& self = *static_cast<CMqtt*>(aSelf);

    self.m_Reinit();

    vTaskDelete(NULL);
}

void CMqtt::m_Reinit() {
    try {
        logger.mInfo("Reiniting client.");

        mDeinit();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        mStartClient();

        logger.mDebug("Client was reinited.\n");
    } catch (const std::exception& e) {
        logger.mError("Could not reinit connection\n");
        App::exit(e);
    }
}

void CMqtt::mDisconnect() {
    if (!mb_Connected) {
        logger.mWarn("Can not disconnect MQTT client, not connected.");
        return;
    }

    m_ReconnectCnt = -1; // overflow
    esp_err_t ret;
    if ((ret = esp_mqtt_client_disconnect(m_Client)) != ESP_OK) throw std::runtime_error("Could not disconnect MQTT client. " + std::string(esp_err_to_name(ret)));
}

void CMqtt::mDeinit() {
    logger.mDebug("Client will deinit now.");

    esp_err_t ret;
    // no need to call disconnect or stop, this is done in esp_mqtt_client_destroy()
    if ((ret = esp_mqtt_client_destroy(m_Client)) != ESP_OK) throw std::runtime_error("Could not destroy MQTT client. " + std::string(esp_err_to_name(ret)));

    m_Client = NULL;

    logger.mDebug("Client was deinited.");
}

void CMqtt::mReconnect() {
    mDisconnect();
    m_Connect();
}

void CMqtt::m_Connect() {
    if (m_Client == NULL) throw std::runtime_error("Client is not started.");
    esp_mqtt_client_reconnect(m_Client);
}

void CMqtt::mStartClient() {
    logger.mDebug("Client will start now.");

    if (!CWifi::getInstance().mConnected()) return;

    if (m_Client != NULL) throw std::runtime_error("Client was already constructed.");

    const esp_mqtt_client_config_t mqtt_cfg = m_GetClientConfig(mcp_AccessToken.c_str());
    m_Client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(m_Client);

    logger.mDebug("Client has started");
}

void CMqtt::mSendMeasurements(Measurements& arValues) {
    if (!mb_Connected) logger.mWarn("Can not send message, client is not connected.");
    if (!mb_Provisioned) logger.mWarn("Can not send message, client is not provisioned.");
    if (!mb_Connected || !mb_Provisioned) return;

    const char* topic = "v1/devices/me/telemetry";

    cJSON* obj = cJSON_CreateObject(); 
    if (obj == NULL) throw std::runtime_error("Could not create measurements object");

    // add all values to 1 layer deep json object
    for (auto& el : arValues) {
        for (auto& p : el.second) {
            // if the json key already exists the value will be skipped
            if (cJSON_GetObjectItemCaseSensitive(obj, p.first.c_str()) == NULL) {
                cJSON_AddNumberToObject(obj, p.first.c_str(), static_cast<double>(p.second));
            }
        }
    }

    const char* publishData = cJSON_Print(obj);
    if (publishData == NULL) m_JsonError(obj, "Could not print json object");

    cJSON_Delete(obj);

    m_SendCustom(topic, publishData);

    if (!mb_SendAttributes) m_SendAttributes(publishData); // also send the data as telemetry
}

void CMqtt::m_SendAttributes(const char* apData) {
    const char* topic = "v1/devices/me/attributes";
    m_SendCustom(topic, apData);

    mb_SendAttributes = true;
}

void CMqtt::m_SendCustom(const char* acpTopic, const char* acpMsg) {
    int msg_id = esp_mqtt_client_publish(m_Client, acpTopic, acpMsg, strlen(acpMsg), 0, 0);
    logger.mDebug("binary sent with msg_id=%d", msg_id);
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
            self.m_ReconnectCnt = 0;

            logger.mInfo("MQTT_EVENT_CONNECTED");

            if (!self.mb_Provisioned) self.m_RequestProvision();
            break;
        case MQTT_EVENT_DISCONNECTED: {
            self.mb_Connected = false;
            self.m_ReconnectCnt++;
            logger.mWarn("MQTT_EVENT_DISCONNECTED");

            if (self.m_ReconnectCnt > 3) {
                logger.mWarn("Reiniting client.");

                // create task to move of the event task
                xTaskHandle handle = NULL;
                xTaskCreate(&m_Reinit, "mqttReinit", CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT, event->user_context, CONFIG_PTHREAD_TASK_PRIO_DEFAULT, &handle);
                return;
            }

            uint8_t timeoutSeconds = pow(self.m_ReconnectCnt, self.m_ReconnectCnt);

            if (self.mb_Provisioned) {
                logger.mDebug("Reconnecting to the server %d/3. Waiting for %d seconds.", self.m_ReconnectCnt, timeoutSeconds);
                vTaskDelay(timeoutSeconds * 1000 / portTICK_PERIOD_MS);
                self.m_Connect();
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
            logger.mInfo("Other event id: %d", event->event_id);
            break;
        }
    }
    catch (const std::runtime_error& err) {
        logger.mError("Error during event: %s.", err.what());
    }
}

void CMqtt::m_RequestProvision() {
    const char* publishTopic = "/provision/request";
    const char* publishData;

    cJSON* obj = cJSON_CreateObject();
    if (obj == NULL) throw std::runtime_error("Could not create provisioning object.");

    if (cJSON_AddStringToObject(obj, "deviceName", mcp_DeviceId.c_str()) == NULL) {
        m_JsonError(obj, "Could not create deviceName for provisioning object.");
    }
    if (cJSON_AddStringToObject(obj, "provisionDeviceKey", PROVISION_KEY) == NULL) {
        m_JsonError(obj, "Could not create provisionDeviceKey for provisioning object.");
    }
    if (cJSON_AddStringToObject(obj, "provisionDeviceSecret", PROVISION_SECRET) == NULL) {
        m_JsonError(obj, "Could not create provisionDeviceSecret for provisioning object.");
    }
    if (cJSON_AddStringToObject(obj, "credentialsType", "ACCESS_TOKEN") == NULL) {
        m_JsonError(obj, "Could not create credentialsType for provisioning object.");
    }
    if (cJSON_AddStringToObject(obj, "token", mcp_AccessToken.c_str()) == NULL) {
        m_JsonError(obj, "Could not create token for provisioning object.");
    }

    publishData = cJSON_Print(obj);
    if (publishData == NULL) m_JsonError(obj, "Could not print json object.");

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
        m_JsonError(data, "Error parsing provisioning JSON.");
    }

    cJSON* status = cJSON_GetObjectItemCaseSensitive(data, "status");
    if (cJSON_IsString(status) && (status->valuestring != NULL)) {
        if (strcmp(status->valuestring, "SUCCESS") == 0) {
            logger.mInfo("Provisioned new device.");
        } else {
            cJSON* errorMsg = cJSON_GetObjectItemCaseSensitive(data, "errorMsg");
            if (cJSON_IsString(errorMsg) && (errorMsg->valuestring != NULL)) {
                if (strcmp(errorMsg->valuestring, "Failed to provision device!") == 0) {
                    logger.mInfo("Device was already provisioned.");
                } else {
                    std::string error("Could not provision device: ");
                    error.append(errorMsg->valuestring);

                    throw std::runtime_error(error);
                }
            } else {
                m_JsonError(data, "Could not parse provision errorMsg.");
            }
        }
    } else {
        m_JsonError(data, "Could not parse provision status.");
    }

    cJSON_Delete(data);

    mb_Provisioned = true;

    xTaskHandle handle = NULL;
    xTaskCreate(&m_Reinit, "mqttReinit", CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT, this, CONFIG_PTHREAD_TASK_PRIO_DEFAULT, &handle);
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

void CMqtt::m_JsonError(cJSON* aJsonObject, const char* aError) {
    cJSON_Delete(aJsonObject);
    throw std::runtime_error(aError);
}
