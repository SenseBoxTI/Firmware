#pragma once
#include "esp_event.h"
#include "mqtt_client.h"
#include <cJSON.h>

class CMqtt {
public:
    CMqtt(const char* acpClientId, const char* acpAssetId);
    void mSendMeasurement(const char* acpAttribute, const char* acpValue);

private:
    static void m_EventHandler(void* apArgs, esp_event_base_t aBase, int32_t aId, void* apData);
    void m_SendCustom(const char* acpTopic, const char* acpMsg);
    void m_Subscribe(const char* acpTopic);
    void m_Unsubscribe(const char* acpTopic);
    void m_RequestProvision();
    void m_OnProvisionResponse(const char* acpData, int aLen);
    void m_JsonError(cJSON* aJsonObject, const char* aError);

    esp_mqtt_client_handle_t m_Client;
    const char* mcp_ClientId;
    const char* mcp_AssetId;
    const char* mcp_Realm;
    const char* mcp_SubscribeTopic;
    std::string m_Cert;
    bool mb_Provisioned;
    bool mb_Connected;
};
