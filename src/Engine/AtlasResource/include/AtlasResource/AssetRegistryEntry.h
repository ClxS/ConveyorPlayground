#pragma once

#include <filesystem>
#include "AtlasCore/Hashing.h"

namespace atlas::resource
{
    struct AssetRegistryEntry
    {
        AssetRegistryEntry(
            std::filesystem::path path,
            const std::string& relativeName)
            : m_Path{std::move(path)}
            , m_RelativeNameHash{core::hashing::fnv1(relativeName)}
#ifdef _DEBUG
            , m_RelativeName{relativeName}
#endif
        {
        }

        std::filesystem::path m_Path;
        uint64_t m_RelativeNameHash;

#ifdef _DEBUG
        std::string m_RelativeName;
#endif
    };
}
