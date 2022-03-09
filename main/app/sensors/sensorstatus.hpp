#pragma once

#include <string>

class CSensorStatus {
    public:
    bool mIsOk( std::string &arReason);
    
    static CSensorStatus Ok();
    static CSensorStatus Error( std::string aReason );
    CSensorStatus();

    private:
    std::string m_Reason;
};