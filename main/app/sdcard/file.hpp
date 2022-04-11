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
    std::string mRead();
    void mWrite(const std::string& aText);
    void mAppend(const std::string& aText);
    size_t mGetFileLength();
    
    CFile(const std::string& aPath);

    std::string mPath;
    
    private:
    void mOpen(FileMode aMode);
    void mClose();

    FileMode m_Mode;
    FILE * m_Ptr;
};
