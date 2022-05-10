#pragma once
#include "esp_event.h"
#include "mqtt_client.h"

class CMqtt {
public:
    CMqtt();

private:
    static void m_EventHandler(void* aArgs, esp_event_base_t aBase, int32_t aId, void* aData);
    static void m_SendBinary(esp_mqtt_client_handle_t aClient);
};
