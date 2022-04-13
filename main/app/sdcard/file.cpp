#include "file.hpp"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <string.h>
#include <iostream>
#include "esp_vfs_fat.h"
#include <stdexcept>

static const char FileModes[3][2] = {
    "r",
    "w",
    "a"
};

CFile::CFile(const std::string& aPath)
:   mPath(aPath),
    m_Mode(Closed),
    m_File(nullptr)
{}

void CFile::m_Open(FileMode aMode) {
    if (m_IsOpen()) m_Close();

    m_File = fopen(mPath.c_str(), FileModes[aMode]);

    if (!m_IsOpen()) throw std::runtime_error("File does not exist and cannot be created\n");

    m_Mode = aMode;
}

void CFile::m_Close() {
    if (m_IsOpen()) fclose(m_File);
    m_File = nullptr;
    m_Mode = Closed;
}

bool CFile::m_IsOpen() {
    return m_File != nullptr;
}

std::string CFile::mRead() {
    size_t size = mGetFileLength();
    if (size <= 0) return "";

    m_Open(Read);

    std::string output;
    output.resize(size);
    for (int i = 0; i < size; i++) output[i] = fgetc(m_File);

    m_Close();

    return output;
}

void CFile::mWrite(const std::string& aText) {
    m_Open(Write);

    fprintf(m_File, aText.c_str());

    m_Close();
}

void CFile::mAppend(const std::string& aText) {
    m_Open(Append);

    fprintf(m_File, aText.c_str());

    m_Close();
}

size_t CFile::mGetFileLength() {
    struct stat stats;

    memset(&stats, 0, sizeof(struct stat));
    stat(mPath.c_str(), &stats);

    return stats.st_size;
}
