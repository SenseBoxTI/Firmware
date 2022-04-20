#include "file.hpp"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <string.h>
#include <iostream>
#include "esp_vfs_fat.h"
#include <stdexcept>
#include "sdmmc_cmd.h"

#define SDSPI_DEFAULT_DMA 3
#define TRANSFER_SIZE 4000
#define SPI_FREQ 5000

#define PIN_CS   4  // CD pin
#define PIN_CLK  2  // SCK pin
#define PIN_MOSI 3
#define PIN_MISO 1

static const char FileModes[3][2] = {
    "r",
    "w",
    "a"
};

SdState CFile::m_SdState = Unitizialized;

CFile::CFile(const std::string& arPath)
:   mPath(MOUNT_POINT "/" + arPath),
    m_Mode(Closed),
    mp_File(nullptr)
{}

void CFile::m_Open(FileMode aMode) {
    if (m_SdState == Unitizialized) throw std::runtime_error("SD card should have been initialized");

    if (m_IsOpen()) m_Close();

    mp_File = fopen(mPath.c_str(), FileModes[aMode]);

    if (!m_IsOpen()) throw std::runtime_error("File does not exist and cannot be created");

    m_Mode = aMode;
}

void CFile::m_Close() {
    if (m_IsOpen())
    {
        if (fclose(mp_File) < 0) {
            throw std::runtime_error("Failed to close file properly");
        }
    }
    mp_File = nullptr;
    m_Mode = Closed;
}

bool CFile::m_IsOpen() {
    return mp_File != nullptr;
}

std::string CFile::mRead() {
    if (m_SdState == Unavailable) return "";

    size_t size = mGetFileLength();
    if (size <= 0) return "";

    m_Open(Read);

    std::string output;
    output.resize(size);
    for (int i = 0; i < size; i++) output[i] = fgetc(mp_File);

    m_Close();

    return output;
}

void CFile::mWrite(const std::string& arText) {
    if (m_SdState == Unavailable) return;

    m_Open(Write);

    fprintf(mp_File, arText.c_str());

    m_Close();
}

void CFile::mAppend(const std::string& arText) {
    if (m_SdState == Unavailable) return;

    m_Open(Append);

    fprintf(mp_File, arText.c_str());

    m_Close();
}

size_t CFile::mGetFileLength() {
    if (m_SdState == Unavailable) return 0;

    struct stat stats;

    memset(&stats, 0, sizeof(struct stat));
    stat(mPath.c_str(), &stats);

    return stats.st_size;
}

CFile::CFile(CFile &&arrOther) {
    mp_File = arrOther.mp_File;
    arrOther.mp_File = nullptr;

    mPath = arrOther.mPath;
}

CFile & CFile::operator=(CFile &&arrOther) {
    mp_File = arrOther.mp_File;
    arrOther.mp_File = nullptr;

    mPath = arrOther.mPath;

    return *this;
}

CFile::~CFile() {
    m_Close();
}

void CFile::mInitSd() {
    m_SdState = Unavailable;

    esp_err_t ret;
    sdmmc_card_t * sdCard;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    std::printf("Initializing SD card\n");
    std::printf("Using SPI peripheral\n");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = SPI_FREQ;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TRANSFER_SIZE,
    };

    std::printf("Initializing bus\n");
    ret = spi_bus_initialize((spi_host_device_t)SDSPI_DEFAULT_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        std::printf("Failed to initialize spi bus.\n");
        throw std::runtime_error(esp_err_to_name(ret));
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = static_cast<gpio_num_t>(PIN_CS);
    slot_config.host_id = static_cast<spi_host_device_t>(host.slot);
    std::printf("Mounting filesystem\n");
    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &sdCard);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            std::printf("Failed to mount filesystem.\n"
                    "If you want the card to be formatted, enable the .format_if_mount_failed in mount config\n");
        } else if (ret == ESP_ERR_INVALID_RESPONSE){
            std::printf("Failed to initialize the card, make sure there is an SD card inserted / connected.\n");
        } else {
            std::printf("Failed to initialize the card (%s).\n"
                    "Make sure SD card lines have pull-up resistors in place.\n", esp_err_to_name(ret));
        }
        throw std::runtime_error(esp_err_to_name(ret));
    }

    std::printf("Filesystem mounted\n");
    m_SdState = Ready;
    sdmmc_card_print_info(stdout, sdCard);
}

SdState CFile::getSdState() {
    return m_SdState;
}
