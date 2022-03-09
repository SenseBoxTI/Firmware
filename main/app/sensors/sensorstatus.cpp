#include "sensorstatus.hpp"

CSensorStatus::CSensorStatus() {}

CSensorStatus CSensorStatus::Ok() {
    CSensorStatus status = {};
    status.m_Reason = "";
    return status;
}

CSensorStatus CSensorStatus::Error( std::string aReason ) {
    CSensorStatus status = {};
    status.m_Reason = aReason;
    return status;
}

bool CSensorStatus::mIsOk( std::string &arReason ) {
    arReason = m_Reason;
    return (m_Reason.compare("") == 0);
}