#pragma once
#include <stdexcept>
#include <iostream>

#include <esp_timer.h>

#define MOUNT_POINT "/sdcard"

enum FileMode { //enumerate for filemode fopen
    Read,
    Write,
    Append,
    Closed
};

enum SdState { //enumerate for filemode fopen
    Unitizialized,
    Ready,
    Unavailable
};

class CFile {
public:
    std::string mPath;

    CFile(const std::string& arPath, FileMode aMode = Read);
    CFile() = delete;

    CFile& operator=(const CFile& arOther) = delete;
    CFile& operator=(CFile&& arrOther);

    CFile(const CFile& arOther) = delete;
    CFile(CFile&& arrOther);

    static void mInitSd();
    static SdState getSdState();
    void mReopen(FileMode aMode);

    std::string mRead();
    void mWrite(const std::string& arText);
    void mAppend(const std::string& arText);
    void mRename(const std::string& arNewName);
    void mDelete();
    bool mExists();
    size_t mGetFileLength();
    ~CFile();

private:
    static SdState m_SdState;

    bool m_IsOpen();
    void m_Open();
    void m_Close();
    static void m_Close(void* aSelf);
    void m_SetCloseTimer();
    std::string m_GetTaskName();

    FileMode m_Mode;
    FILE* mp_File;
    esp_timer_handle_t m_CloseTimer;
};
