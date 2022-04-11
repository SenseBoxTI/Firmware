#include "file.hpp"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <string.h>
#include <iostream>
#include "esp_vfs_fat.h"

static const char FileModes[3][2] = {
    "r",
    "w",
    "a"
};

CFile::CFile(const std::string& aPath) : mPath(aPath) {
    m_Ptr = nullptr;
    m_Mode = FileMode::Write;
}

void CFile::mOpen(FileMode aMode) {
    if (mIsOpen()){ // when already opened
        if (!(aMode == m_Mode && aMode == FileMode::Append)) { // when appen don't reopen, else do it
            mClose();
            m_Ptr = fopen(mPath.c_str(), FileModes[aMode]);
        }
    }
    else{ // when not opened
        m_Ptr = fopen(mPath.c_str(), FileModes[aMode]);
    }

    m_Mode = aMode;
}

void CFile::mClose() {
    if (mIsOpen()) 
        fclose(m_Ptr);
    m_Ptr = nullptr;
}

bool CFile::mIsOpen() {
    return m_Ptr != nullptr;
}

std::string CFile::mRead(){
    mOpen(Read);
    std::string output;
    size_t size = mGetFileLength();
    output.resize(size);
    for (int i = 0; i < size; i++) {
        output[i] = fgetc(m_Ptr);
    }
    return output;
}

void CFile::mWrite(const std::string& aText) {
    mOpen(Write);
    fprintf(m_Ptr, aText.c_str());
}

void CFile::mAppend(const std::string& aText) {
    mOpen(Append);
    fprintf(m_Ptr, aText.c_str());
}

size_t CFile::mGetFileLength() {
    struct stat stats;
    stat(mPath.c_str(), &stats);
    return stats.st_size;
}
