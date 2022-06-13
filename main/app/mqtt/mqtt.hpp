#pragma once
#include "esp_event.h"
#include "mqtt_client.h"
#include <cJSON.h>
#include <measurements.hpp>

class CMqtt {
public:
    void mInit(const std::string& acrDeviceId, const std::string& acrAccessToken);
    static CMqtt& getInstance();
    void mSendMeasurements(Measurements& arValues);
    void mDisconnect();
    void mReconnect();

private:
    CMqtt();
    static void m_EventHandler(void* apArgs, esp_event_base_t aBase, int32_t aId, void* apData);
    void m_SendCustom(const char* acpTopic, const char* acpMsg);
    void m_Subscribe(const char* acpTopic);
    void m_Unsubscribe(const char* acpTopic);
    void m_RequestProvision();
    void m_OnProvisionResponse(const char* acpData, int aLen);
    void m_JsonError(cJSON* aJsonObject, const char* aError);
    esp_mqtt_client_config_t m_GetClientConfig(const char* aUsername);
    void m_Connect();

    esp_mqtt_client_handle_t m_Client;
    std::string mcp_DeviceId;
    std::string mcp_AccessToken;
    const char* mcp_SubscribeTopic;
    bool mb_Provisioned;
    bool mb_Connected;
    bool mb_SendAttributes;
};
