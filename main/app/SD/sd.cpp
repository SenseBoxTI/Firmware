#include "sd.hpp"
#include <sys/stat.h>
#include <sys/unistd.h>
#include <string.h>
#include <iostream>
#include "esp_vfs_fat.h"

#define SDSPI_DEFAULT_DMA 1
static constexpr const char* mount_point = MOUNT_POINT;
static constexpr const int pin_miso = 6;
static constexpr const int pin_mosi = 4;
static constexpr const int pin_clk = 5;
static constexpr const int pin_cs = 1;

CSD& CSD::getInstance() {
    static CSD instance = {};
    return instance;
}

esp_err_t CSD::mInit() {
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = pin_mosi,
        .miso_io_num = pin_miso,
        .sclk_io_num = pin_clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        std::printf("Failed to initialize spi bus.");
        return ret;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = static_cast<gpio_num_t>(pin_cs);
    slot_config.host_id = static_cast<spi_host_device_t>(host.slot);
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            std::printf("Failed to mount filesystem. "
                     "If you want the card to be formatted, enable the .format_if_mount_failed in mount config");
        } else {
            std::printf("Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ret;
    }

    return ret;
}