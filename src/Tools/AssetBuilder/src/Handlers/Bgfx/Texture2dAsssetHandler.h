#pragma once
#include <filesystem>
#include <string>

#include "Asset.h"
#include "CopyAssetHandler.h"
#include "tomlcpp.hpp"

struct Asset;

class Texture2DAssetHandler final : public AssetHandler
{
public:
    static std::string GetAssetTag() { return "texture2d"; }
    std::filesystem::path GetAssetRelativeOutputPath(const Asset& fullPath) override
    {
        auto outputPath = fullPath.m_RelativePath;
        return outputPath.replace_extension("ntx2d");
    }

    std::variant<std::vector<OutputArtifact>, ErrorString> Cook(const Asset& asset) override;
};
