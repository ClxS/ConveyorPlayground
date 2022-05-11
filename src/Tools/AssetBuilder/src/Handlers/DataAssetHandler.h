#pragma once
#include <filesystem>
#include <string>

#include "Asset.h"
#include "CopyAssetHandler.h"
#include "tomlcpp.hpp"

struct Asset;

class DataAssetHandler final : public CopyAssetHandler
{
public:
    static std::string GetAssetTag() { return "data"; }
    std::filesystem::path GetCopiedDataPath(const toml::Table* root) override
    {
        return root->getString("source").second;
    }
    std::filesystem::path GetAssetRelativeOutputPath(const Asset& fullPath) override
    {
        auto outputPath = fullPath.m_RelativePath;
        return outputPath.replace_extension("ndata");
    }

protected:
};
