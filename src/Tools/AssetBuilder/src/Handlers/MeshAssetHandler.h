#pragma once
#include "AssetHandler.h"

namespace toml
{
    class Table;
}

class MeshAssetHandler : public AssetHandler
{
public:
    static std::string GetAssetTag() { return "mesh"; }
    std::filesystem::path GetAssetRelativeOutputPath(const Asset& fullPath) override;

    std::variant<std::vector<OutputArtifact>, ErrorString> Cook(const Asset& asset) override;
};
