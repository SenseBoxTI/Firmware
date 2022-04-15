#pragma once
#include <stdexcept>
#include <iostream>

enum FileMode { //enumerate for filemode fopen
    Read,
    Write,
    Append,
    Closed
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

    std::string mRead();
    void mWrite(const std::string& arText);
    void mAppend(const std::string& arText);
    size_t mGetFileLength();

    private:
    ~CFile();

    bool m_IsOpen();
    void m_Open(FileMode aMode);
    void m_Close();

    FileMode m_Mode;
    FILE* m_File;
};
