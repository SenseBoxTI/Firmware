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

//aPath = "/sdcard/sd_test.txt";

CFile::CFile(std::string aPath) {
    mPath = aPath;
}

void CFile::mOpen(FileMode aMode){
    if (m_Ptr != NULL){ // wanneer bestaat
        if (aMode != m_Mode){//wanneer mode niet klopt
        mClose();
        m_Ptr = fopen(mPath.c_str(), FileModes[aMode]);
        }
    }
    else{ // wanneer niet open
    m_Mode = aMode;
    m_Ptr = fopen(mPath.c_str(), FileModes[aMode]);
    }
}

void CFile::mClose(){
    fclose(m_Ptr);
    m_Ptr = NULL;
}

bool CFile::mIsOpen(){
    if (m_Ptr != NULL){
        return true;
    }
    else {
        return false;
    }
}

void CFile::mClear(){
    if (mIsOpen()){ 
        mClose();
    }
    mOpen(Write); // clears entire file 
    mClose();
}

std::string CFile::mRead(){
    char ch;
    std::string output;
    mOpen(Read);
    std::printf("Printing out sd_test.txt:\n");
    while(fgets(&ch, 1, m_Ptr) != NULL)
        output += ch;
    return output;
    
}

void CFile::mWrite(std::string aText){
    mOpen(Write);
    fprintf(m_Ptr,"%s",aText.c_str());
}

void CFile::mAppend(std::string aText){
    mOpen(Append);
    fprintf(m_Ptr,"%s",aText.c_str());
}
