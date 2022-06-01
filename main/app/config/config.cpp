#include "config.hpp"

CConfig::CConfig() {

}

CConfig& CConfig::getInstance() {
    static CConfig instance = {};
    return instance;
}

void CConfig::mRead(const std::string& ar_FilePath) {
    auto parsedFile = toml::parseFile(ar_FilePath);
    
    // if we fail to parse config, throw error!
    if (!parsedFile.valid()) {
        throw std::runtime_error(parsedFile.errorReason);
    }
    
    m_Config = parsedFile.value;
}
