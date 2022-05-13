#pragma once
#include <dirent.h>
#include <vector>
#include <file.hpp>

class CDir {
    public:
        std::string mPath;

        CDir(const std::string& arPath);

        std::vector<CFile> mListFiles();
        CFile mFile(const std::string& arFileName);
        void mEnsure();
    private:
        void m_Open();
        void m_Close();
        std::string m_MountPath();

        DIR* m_Dir;
};
