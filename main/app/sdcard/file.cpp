#include "file.hpp"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <string.h>
#include <iostream>
#include <stdexcept>
#include "sdmmc_cmd.h"
#include <CConfig.hpp>
#include <freertos/portmacro.h>

#include <logscope.hpp>

#define SDSPI_DEFAULT_DMA 3
#define TRANSFER_SIZE 4000
#define SPI_FREQ 5000

#define PIN_CS   4  // CD pin
#define PIN_CLK  2  // SCK pin
#define PIN_MOSI 3
#define PIN_MISO 1

#define QUEUE_SIZE_BYTES 128

static CLogScope logger{"file"};

static const char FileModes[3][2] = {
    "r",
    "w",
    "a"
};

SdState CFile::m_SdState = Unitizialized;
sdmmc_card_t* CFile::m_SdCard = nullptr;

CFile::CFile(const std::string& arPath, FileMode aMode)
:   mPath(MOUNT_POINT "/" + arPath),
    m_Mode(aMode),
    mp_File(nullptr),
    m_FileUntouchedCnt(0),
    m_WriteTimer(nullptr)
{
    taskName = m_GetTaskName();
    m_WriteTimer = CTimer::mInit(taskName.c_str(), &m_StartWrite, this);
    m_WriteTimer->mStartPeriodic(FILE_WRITE_INTERVAL);

    m_WriteQueue = xQueueCreate(16, QUEUE_SIZE_BYTES); // queue of 20
}

/**
 * Not thread safe if the file is being written to
 * But currently all tasks have the same prio so there should not be any context switching
 */
void CFile::mReopen(FileMode aMode) {
    if (aMode == m_Mode) return;

    mFlushQueue();
    m_Close();

    m_Mode = aMode;

    m_Open();
}

void CFile::m_Open() {
    // do nothing if file is already opened
    if (m_IsOpen()) return;

    if (m_SdState == Unitizialized) throw std::runtime_error("SD card is not initialized");

    mp_File = fopen(mPath.c_str(), FileModes[m_Mode]);
    m_FileUntouchedCnt = 0;

    if (!m_IsOpen()) throw std::runtime_error("File does not exist and cannot be created");
}

void CFile::m_Close() {
    if (!m_IsOpen()) return;

    if (fclose(mp_File) < 0) {
        throw std::runtime_error("Failed to close file properly");
    }

    mp_File = nullptr;
}

bool CFile::m_IsOpen() {
    return mp_File != nullptr;
}

std::string CFile::mRead() {
    if (m_SdState == Unavailable) return "";
    if (m_Mode != Read) std::runtime_error("File " + mPath + " is not opened in read mode");

    size_t size = mGetFileLength();
    if (size <= 0) return "";

    m_Open();

    std::string output;
    output.resize(size);
    for (int i = 0; i < size; i++) output[i] = fgetc(mp_File);

    m_Close();

    return output;
}

void CFile::mWrite(const std::string& arText) {
    if (m_SdState == Unavailable) return;
    if (m_Mode != Write) std::runtime_error("File " + mPath + " is not opened in write mode");

    m_Open();

    fprintf(mp_File, arText.c_str());

    // reset the counter
    m_FileUntouchedCnt = 0;
}

void CFile::mAppend(const std::string& arText) {
    if (m_SdState == Unavailable) return;
    if (m_Mode != Append) std::runtime_error("File " + mPath + " is not opened in append mode");

    const size_t strLen = arText.size();

    if (strLen <= QUEUE_SIZE_BYTES) {
        m_AddToQueue(arText.c_str());
        return;
    }

    // add multiple items to the queue if the text is too long
    for (size_t i = 0; i < strLen; i += QUEUE_SIZE_BYTES) {
        m_AddToQueue(arText.substr(i, QUEUE_SIZE_BYTES).c_str());
    }
}

/**
 * Add item to the write queue
 */
void CFile::m_AddToQueue(const char* apText) {
    if (strlen(apText) > QUEUE_SIZE_BYTES) throw std::runtime_error("Text is longer than " + std::to_string(QUEUE_SIZE_BYTES) + " characters.");

    BaseType_t result;
    BaseType_t isIsr = xPortInIsrContext();

    if (isIsr)  result = xQueueSendToBackFromISR(m_WriteQueue, apText, (TickType_t)0);
    else        result = xQueueSendToBack(m_WriteQueue, apText, (TickType_t)0);

    if (result == errQUEUE_FULL) {
        m_WriteFromQueue();

        if (isIsr)  xQueueSendToBackFromISR(m_WriteQueue, apText, (TickType_t)0);
        else        xQueueSendToBack(m_WriteQueue, apText, (TickType_t)0);
    }
}

bool CFile::mExists() {
    return (m_SdState != Unavailable) && (!access(mPath.c_str(), F_OK));
}

void CFile::mRename(const std::string& arNewName) {
    if (!mExists()) return;

    std::string oldPath = mPath;
    mPath = mPath.substr(0, mPath.find_last_of('/') + 1) + arNewName;
    rename(oldPath.c_str(), mPath.c_str());
}

void CFile::mDelete() {
    if (mExists()) remove(mPath.c_str());
}

/**
 * purely a wrapper of m_WriteFromQueue() to satisfy the esp_timer callback
 */
void CFile::m_StartWrite(void* aSelf) {
    CFile& self = *static_cast<CFile*>(aSelf);
    self.m_WriteFromQueue();
}

/**
 * Writes the messages from the queue to the log file
 */
void CFile::m_WriteFromQueue() {
    static const uint8_t maxUntouchedBeforeClose = FILE_CLOSE_TIMEOUT / FILE_WRITE_INTERVAL;

    if (uxQueueMessagesWaiting(m_WriteQueue) > 0) {
        m_Open();

        char buffer[QUEUE_SIZE_BYTES];

        while (uxQueueMessagesWaiting(m_WriteQueue) > 0) {
            if (xQueueReceive(m_WriteQueue, &buffer, (TickType_t)0)) {
                fprintf(mp_File, buffer);
            }
        }

        m_FileUntouchedCnt = 0;
    } else if (m_FileUntouchedCnt == maxUntouchedBeforeClose) {
        m_FileUntouchedCnt = 0;
        m_Close();
    } else if (m_IsOpen()) {
        m_FileUntouchedCnt++;
    }
}

/**
 * Write any data that is still pending from the queue and close the file
 */
void CFile::mFlushQueue() {
    if (m_Mode != Append) return;
    if (!m_IsOpen()) return;

    char buffer[QUEUE_SIZE_BYTES];

    while (uxQueueMessagesWaiting(m_WriteQueue) > 0) {
        if (xQueueReceive(m_WriteQueue, &buffer, (TickType_t)0)) {
            fprintf(mp_File, buffer);
        }
    }

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
    m_WriteTimer->mDelete();
    vQueueDelete(m_WriteQueue);
}

std::string CFile::m_GetTaskName() {
    const size_t len = mPath.size();
    const size_t maxLen = CONFIG_FREERTOS_MAX_TASK_NAME_LEN;

    if (len <= maxLen) return mPath;
    return mPath.substr(len - maxLen);
}

void CFile::mInitSd() {
    m_SdState = Unavailable;

    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    logger.mInfo("Initializing SD card");
    logger.mDebug("Using SPI peripheral");
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

    logger.mDebug("Initializing bus");
    ret = spi_bus_initialize((spi_host_device_t)SDSPI_DEFAULT_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        logger.mError("Failed to initialize spi bus.");
        throw std::runtime_error(esp_err_to_name(ret));
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = static_cast<gpio_num_t>(PIN_CS);
    slot_config.host_id = static_cast<spi_host_device_t>(host.slot);
    logger.mDebug("Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &m_SdCard);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            logger.mError("Failed to mount filesystem.");
            logger.mError("If you want the card to be formatted, enable the .format_if_mount_failed in mount config");
        } else if (ret == ESP_ERR_INVALID_RESPONSE){
            logger.mError("Failed to initialize the card, make sure there is an SD card inserted / connected.");
        } else {
            logger.mError("Failed to initialize the card (%s).");
            logger.mError("Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        throw std::runtime_error(esp_err_to_name(ret));
    }

    logger.mDebug("Filesystem mounted");
    m_SdState = Ready;
    sdmmc_card_print_info(stdout, m_SdCard);
}

void CFile::mDeinitSd() {
    m_SdState = Unavailable;

    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, m_SdCard);
    spi_bus_free((spi_host_device_t)SDSPI_DEFAULT_HOST);

    logger.mInfo("File system has been unmounted\n");
}

SdState CFile::getSdState() {
    return m_SdState;
}
