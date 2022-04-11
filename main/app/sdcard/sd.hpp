#pragma once
#include "sdmmc_cmd.h"
#include "file.hpp"

#define MOUNT_POINT "/sdcard"

class CSD {
    sdmmc_card_t * m_Card;
    public:
        
        static CSD& getInstance();
        void mInit();
};
