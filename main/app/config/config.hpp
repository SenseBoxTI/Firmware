#pragma once

#include <toml.h>
#include <file.hpp>

class CConfig {
    public:
        [[nodiscard]] static CConfig& getInstance();

        void mRead(const std::string& ar_FilePath);

        [[nodiscard]] inline toml::Value& operator[](const std::string& key) { return m_config[key]; }

        template<typename T>
        [[nodiscard]] inline T get(const std::string& key) { return m_config.get<T>(key); }

    private:
        toml::Value m_config;
        CConfig();
};