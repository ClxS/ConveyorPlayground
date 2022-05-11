#pragma once
#include <filesystem>

struct Asset
{
    std::filesystem::path m_SourceAssetPath;
    std::filesystem::path m_RelativePath;
    class AssetHandler* m_pAssociatedHandler;
    std::string m_Type;
};
