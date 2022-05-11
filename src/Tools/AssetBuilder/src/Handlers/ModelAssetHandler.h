#pragma once
#include "AssetHandler.h"

namespace toml
{
    class Table;
}

class ModelAssetHandler : public AssetHandler
{
public:
    static std::string GetAssetTag() { return "model"; }
    std::filesystem::path GetAssetRelativeOutputPath(const Asset& fullPath) override;

    std::variant<std::vector<OutputArtifact>, ErrorString> Cook(const Asset& asset) override;
};
