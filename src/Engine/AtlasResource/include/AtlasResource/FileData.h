#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>

#include "RegistryId.h"

namespace atlas::resource
{
    struct FileData
    {
        std::filesystem::path m_FilePath;
        RegistryId m_RegistryId;
        std::unique_ptr<uint8_t[]> m_pData;
        uint64_t m_Size;
    };
}
