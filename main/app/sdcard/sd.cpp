#include "sd.hpp"
#include "file.hpp"
#include <sys/stat.h>
#include <sys/unistd.h>
#include <string.h>
#include <iostream>
#include "esp_vfs_fat.h"

#define SDSPI_DEFAULT_DMA 1

// TODO: fix numbers for esp32-s3
#define PIN_CS 19
#define PIN_CLK 18
#define PIN_MOSI 5
#define PIN_MOSI 17

CSD& CSD::getInstance() {
    static CSD instance = {};
    return instance;
}

void CSD::mInit() {
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    std::printf("Initializing SD card\n");
    std::printf("Using SPI peripheral\n");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = 5000;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MOSI,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
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
    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &m_Card);
    
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            std::printf("Failed to mount filesystem. \n"
                     "If you want the card to be formatted, enable the .format_if_mount_failed in mount config\n");
        } else if (ret == ESP_ERR_INVALID_RESPONSE){
            std::printf("Failed to initialize the card, make sure there is an SD card inserted / connected.\n");
        } else {
            std::printf("Failed to initialize the card (%s). \n"
                     "Make sure SD card lines have pull-up resistors in place.\n", esp_err_to_name(ret));
        }
        throw std::runtime_error(esp_err_to_name(ret));
    }
    
    std::printf("Filesystem mounted\n");
    sdmmc_card_print_info(stdout, m_Card);
}
