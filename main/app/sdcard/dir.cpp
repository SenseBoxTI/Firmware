#include "dir.hpp"
#include <sys/stat.h>
#include <errno.h>
#include <logscope.hpp>

static CLogScope logger{"dir"};

CDir::CDir(const std::string& arPath) 
:   mPath(arPath), 
    m_Dir(nullptr)
{
    // ensure path does not end in /
    if (mPath.back() == '/')
        mPath.resize(mPath.size() - 1);
}

std::vector<CFile> CDir::mListFiles()
{
    m_Open();
    struct dirent* dirEntry = nullptr;
    std::vector<CFile> result;

    while ((dirEntry = readdir(m_Dir)) != nullptr) {
        // if file type is not a file, skip this entry
        if (dirEntry->d_type != DT_REG)
            continue;
        std::string fullPath = mPath + "/" + dirEntry->d_name;
        result.emplace_back(fullPath);
    }

    m_Close();
    return result;
}

void CDir::mEnsure()
{
    errno = 0;
    if (mkdir(m_MountPath().c_str(), S_IRWXU) == -1)
    {
        switch (errno) {
            case EACCES:
                throw std::runtime_error("Parent directory does not allow writing");
            case ENAMETOOLONG:
                throw std::runtime_error("Directory pathname was too long");
            default:
                throw std::runtime_error("Unknown mkdir error: " + std::to_string(errno));
            case EEXIST:
                break;
        }
    }
}

void CDir::m_Open()
{
    mEnsure();
    m_Close();
    m_Dir = opendir(m_MountPath().c_str());
    if (!m_Dir) throw std::runtime_error("Could not open directory");
}

void CDir::m_Close()
{
    if (m_Dir) closedir(m_Dir);
    m_Dir = nullptr;
}

std::string CDir::m_MountPath() {
    return MOUNT_POINT "/" + mPath;
}

CFile CDir::mFile(const std::string& arFileName) {
    mEnsure();
    return CFile(mPath + "/" + arFileName);
}
