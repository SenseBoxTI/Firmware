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

static CLogScope logger{"mqtt"};

void CMqtt::m_EventHandler(void* aArgs, esp_event_base_t aBase, int32_t aId, void* aData) {
    logger.mDebug("Event dispatched from event loop base=%s, event_id=%d", aBase, aId);
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(aData);
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)aId) {
    case MQTT_EVENT_CONNECTED:
        logger.mInfo("MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        logger.mInfo("sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        logger.mInfo("sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        logger.mInfo("sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        logger.mInfo("MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        logger.mInfo("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        logger.mInfo("sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        logger.mInfo("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        logger.mInfo("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        logger.mInfo("MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        if (strncmp(event->data, "send binary please", event->data_len) == 0) {
            logger.mInfo("Sending the binary");
            m_SendBinary(client);
        }
        break;
    case MQTT_EVENT_ERROR:
        logger.mInfo("MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            logger.mInfo("Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            logger.mInfo("Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            logger.mInfo("Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                    strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            logger.mInfo("Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            logger.mWarn("Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        logger.mInfo("Other event id:%d", event->event_id);
        break;
    }
}

void CMqtt::m_SendBinary(esp_mqtt_client_handle_t aClient) {
    spi_flash_mmap_handle_t out_handle;
    const void *binary_address;
    const esp_partition_t *partition = esp_ota_get_running_partition();
    esp_partition_mmap(partition, 0, partition->size, SPI_FLASH_MMAP_DATA, &binary_address, &out_handle);
    // sending only the configured portion of the partition (if it's less than the partition size)
    // int binary_size = MIN(CONFIG_BROKER_BIN_SIZE_TO_SEND, partition->size);
    int msg_id = esp_mqtt_client_publish(aClient, "/topic/binary", reinterpret_cast<const char *>(binary_address), partition->size, 0, 0);
    logger.mInfo("binary sent with msg_id=%d", msg_id);
}