#pragma once 
#include "sdmmc_cmd.h"

#define MOUNT_POINT "/sdcard"
class CSD {
    sdmmc_card_t* m_card = nullptr;
    bool m_inited = false;
    public:
        
        /// @brief this class is a singleton, get the instance of this class that is used.
        static CSD& getInstance();

        /// @brief initialize the SD card over SPI
        /// @return ESP_OK if successful, otherwise an error code
        esp_err_t mInit();

        /// @brief gets if the SD card was inited properly
        /// @return true for yes, false for no
        bool get_inited();
};