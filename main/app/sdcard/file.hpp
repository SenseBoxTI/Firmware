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

    std::string mRead();
    void mWrite(const std::string& arText);
    void mAppend(const std::string& arText);
    size_t mGetFileLength();

    private:
    CFile();
    bool m_IsOpen();
    void m_Open(FileMode aMode);
    void m_Close();

    FileMode m_Mode;
    FILE* m_File;
};
