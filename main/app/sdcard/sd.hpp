#pragma once
#include "sdmmc_cmd.h"

#define MOUNT_POINT "/sdcard"

class CSd {
    sdmmc_card_t * m_Card;
    public:
        static CSd& getInstance();
        void mInit();
};
