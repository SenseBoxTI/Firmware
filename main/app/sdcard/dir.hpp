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
    private:
        void m_Open();
        void m_Close();
        void m_Ensure();
        std::string m_MountPath();

        DIR* m_Dir;
};
