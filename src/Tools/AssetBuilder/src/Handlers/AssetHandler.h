#pragma once
#include <filesystem>
#include <variant>

#include "OutputArtifact.h"

struct Asset;
using ErrorString = std::string;

class AssetHandler
{
public:
    virtual ~AssetHandler() = default;

    virtual std::filesystem::path GetAssetRelativeOutputPath(const Asset& asset) = 0;
    virtual std::variant<std::vector<OutputArtifact>, ErrorString> Cook(const Asset& asset) = 0;
};
