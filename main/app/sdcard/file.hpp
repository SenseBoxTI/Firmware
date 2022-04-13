#pragma once
#include <stdexcept>
#include <iostream>

enum FileMode { //enumerate for filemode fopen
    Read,
    Write,
    Append,
};

class CFile {
    public:
    std::string mPath;

    CFile(const std::string& aPath);

    bool mIsOpen();
    std::string mRead();
    void mWrite(const std::string& aText);
    void mAppend(const std::string& aText);
    size_t mGetFileLength();

    private:
    void m_Open(FileMode aMode);
    void m_Close();

    FileMode m_Mode;
    FILE* m_Ptr;
};
