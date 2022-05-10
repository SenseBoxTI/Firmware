#pragma once
#include <stdexcept>
#include <iostream>

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

    CFile(const std::string& arPath);
    CFile() = delete;

    CFile& operator=(const CFile& arOther) = delete;
    CFile& operator=(CFile&& arrOther);

    CFile(const CFile& arOther) = delete;
    CFile(CFile&& arrOther);

    static void mInitSd();
    static SdState getSdState();

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
    void m_Open(FileMode aMode);
    void m_Close();

    FileMode m_Mode;
    FILE* mp_File;
};
