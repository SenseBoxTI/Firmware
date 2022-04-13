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

void CFile::m_Open(FileMode aMode) {
    if (mIsOpen()) { // when already opened
        if (!(aMode == m_Mode && aMode == FileMode::Append)) { // when appen don't reopen, else do it
            m_Close();
            m_Ptr = fopen(mPath.c_str(), FileModes[aMode]);
        }
    } else { // when not opened
        m_Ptr = fopen(mPath.c_str(), FileModes[aMode]);
    }

    m_Mode = aMode;
}

void CFile::m_Close() {
    if (mIsOpen()) fclose(m_Ptr);
    m_Ptr = nullptr;
}

bool CFile::mIsOpen() {
    return m_Ptr != nullptr;
}

std::string CFile::mRead() {
    size_t size = mGetFileLength();
    if (size <= 0) return "";
    m_Open(Read);

    std::string output;
    output.resize(size);
    for (int i = 0; i < size; i++) output[i] = fgetc(m_Ptr);
    return output;
}

void CFile::mWrite(const std::string& aText) {
    m_Open(Write);
    fprintf(m_Ptr, aText.c_str());
}

void CFile::mAppend(const std::string& aText) {
    m_Open(Append);
    fprintf(m_Ptr, aText.c_str());
}

size_t CFile::mGetFileLength() {
    struct stat stats;
    memset(&stats, 0, sizeof(struct stat));
    stat(mPath.c_str(), &stats);
    return stats.st_size;
}
