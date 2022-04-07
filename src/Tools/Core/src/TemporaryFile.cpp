#include "Utility/TemporaryFile.h"

#include <filesystem>
#include <mutex>

static std::mutex s_fileCreationMutex;

cpp_conv::tools::utility::TemporaryFile::TemporaryFile()
    //: m_File{nullptr}
{
    std::lock_guard<std::mutex> lock(s_fileCreationMutex);
    m_Path = std::tmpnam(nullptr);
    //m_File = std::tmpfile();
}

cpp_conv::tools::utility::TemporaryFile::~TemporaryFile()
{
    std::filesystem::remove(m_Path);
}
