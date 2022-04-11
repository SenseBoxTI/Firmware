#pragma once
#include <stdexcept>
#include <iostream>

enum FileMode{ //enumerate for filemode fopen
Read,
Write,
Append,
};

class CFile {

    public:
    bool mIsOpen();
    void mClear();
    std::string mRead();
    void mWrite(std::string aText);
    void mAppend(std::string aText);
    CFile(std::string aPath);
    std::string mPath;

    private:
    void mOpen(FileMode aMode);
    void mClose();
    FileMode m_Mode;
    FILE * m_Ptr;

};
