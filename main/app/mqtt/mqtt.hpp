#pragma once
#include "esp_event.h"
#include "mqtt_client.h"
#include <cJSON.h>
#include <measurements.hpp>

class CMqtt {
public:
    void mInit(const char* acpDeviceId, const char* acpAccessToken);
    static CMqtt& getInstance();
    void mSendMeasurements(Measurements& arValues);

private:
    CMqtt();
    static void m_EventHandler(void* apArgs, esp_event_base_t aBase, int32_t aId, void* apData);
    void m_SendCustom(const char* acpTopic, const char* acpMsg);
    void m_Subscribe(const char* acpTopic);
    void m_Unsubscribe(const char* acpTopic);
    void m_RequestProvision();
    void m_OnProvisionResponse(const char* acpData, int aLen);
    void m_JsonError(cJSON* aJsonObject, const char* aError);
    void m_Reconnect();

    esp_mqtt_client_handle_t m_Client;
    const char* mcp_DeviceId;
    const char* mcp_AccessToken;
    const char* mcp_SubscribeTopic;
    std::string m_Cert;
    bool mb_Provisioned;
    bool mb_Connected;
    bool mb_SendAttributes;
};
