#include "sdtest.hpp"
#include <sd.hpp>
#include <file.hpp>

CFile file = CFile("/sdcard/sdtest.cbt");

SensorOutput CSdTest::m_MeasureCallback() {
    SensorOutput output;
    output.insert({"Should say 'Success' ->", file.mRead()});
    return output;
}

CSensorStatus CSdTest::m_InitCallback() {
    try {
        CSd::getInstance().mInit();
        file.mWrite("Success");
    }
    catch (const std::runtime_error& e) {
        return CSensorStatus::Error(e.what() + errno);
    }
    return CSensorStatus::Ok();
}