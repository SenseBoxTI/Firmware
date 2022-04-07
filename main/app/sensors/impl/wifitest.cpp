#include "wifitest.hpp"
#include <wifi.hpp>

SensorOutput CWifiTest::m_MeasureCallback() {
    SensorOutput output;
    output.insert({"ConnectionTest", (CWifi::getInstance().mConnected()) ? "Success" : "Failed"});
    return output;
}

CSensorStatus CWifiTest::m_InitCallback() {
    try {
        CWifi::getInstance().mInitWifi({
            .ssid = "SSID",
            .password = "PASSWORD"
        });
    }
    catch (const std::runtime_error &e) {
        std::printf("Error thrown while initing wifi: %s", e.what());
    }
    return CSensorStatus::Ok();
}