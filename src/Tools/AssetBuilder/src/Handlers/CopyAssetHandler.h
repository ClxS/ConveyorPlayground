#pragma once
#include "AssetHandler.h"

namespace toml
{
    class Table;
}

class CopyAssetHandler : public AssetHandler
{
public:
    std::filesystem::path GetAssetRelativeOutputPath(const Asset& fullPath) override;

    std::variant<std::vector<OutputArtifact>, ErrorString> Cook(const Asset& asset) override;

protected:
    virtual std::filesystem::path GetCopiedDataPath(const toml::Table* root) = 0;
};
