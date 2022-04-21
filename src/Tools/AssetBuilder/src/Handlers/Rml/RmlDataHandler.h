#pragma once
#include <filesystem>
#include <string>

#include "Asset.h"
#include "CopyAssetHandler.h"
#include "tomlcpp.hpp"

struct Asset;

class RmlPageDataHandler final : public CopyAssetHandler
{
public:
    static std::string GetAssetTag() { return "rmlPage"; }

    std::filesystem::path GetCopiedDataPath(const toml::Table* root) override
    {
        return root->getString("source").second;
    }

    std::filesystem::path GetAssetRelativeOutputPath(const Asset& fullPath) override
    {
        auto outputPath = fullPath.m_RelativePath;
        return outputPath.replace_extension("rmlpage");
    }
};

class RmlFontDataHandler final : public CopyAssetHandler
{
public:
    static std::string GetAssetTag() { return "rmlFont"; }

    std::filesystem::path GetCopiedDataPath(const toml::Table* root) override
    {
        return root->getString("source").second;
    }

    std::filesystem::path GetAssetRelativeOutputPath(const Asset& fullPath) override
    {
        auto outputPath = fullPath.m_RelativePath;
        return outputPath.replace_extension("rmlttf");
    }
};

class RmlCssDataHandler final : public CopyAssetHandler
{
public:
    static std::string GetAssetTag() { return "rmlCss"; }

    std::filesystem::path GetCopiedDataPath(const toml::Table* root) override
    {
        return root->getString("source").second;
    }

    std::filesystem::path GetAssetRelativeOutputPath(const Asset& fullPath) override
    {
        auto outputPath = fullPath.m_RelativePath;
        return outputPath.replace_extension("rcss");
    }
};

