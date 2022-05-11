#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

struct OutputArtifact
{
    std::unique_ptr<uint8_t[]> m_pData;
    size_t m_DataSize;
    std::filesystem::path m_OutputPath;
};
