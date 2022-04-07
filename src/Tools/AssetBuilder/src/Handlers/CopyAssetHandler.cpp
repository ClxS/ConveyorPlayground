#include "CopyAssetHandler.h"
#include "Asset.h"
#include <fstream>

#include "Cook.h"
#include "FileUtility.h"
#include "tomlcpp.hpp"

std::filesystem::path CopyAssetHandler::GetAssetRelativeOutputPath(const Asset& fullPath)
{
    return fullPath.m_RelativePath;
}

std::variant<std::vector<OutputArtifact>, ErrorString> CopyAssetHandler::Cook(const Asset& asset)
{
    const auto result = asset_builder::utility::file_utility::readTomlFromFile(asset.m_SourceAssetPath);
    if (std::holds_alternative<std::string>(result))
    {
        return std::format("Failed to read TOML: {} - {}\n", std::get<1>(result), asset.m_SourceAssetPath.string());
    }

    auto table = std::get<0>(result);
    const auto keys = table->keys();
    if (keys.size() != 1)
    {
        return std::format("Resource metadata tables can only contain a maximum of 1 root level keys. File {} contains {}", asset.m_SourceAssetPath.string(), keys.size());
    }

    const auto rootTable = table->getTable(keys[0]);
    if (!rootTable)
    {
        return std::format("Root item was not a table. File {}", asset.m_SourceAssetPath.string());
    }

    std::vector<OutputArtifact> outputFiles;
    {
        auto parent = asset.m_SourceAssetPath.parent_path();
        auto copiedArtifactPath = parent / GetCopiedDataPath(rootTable.get());
        std::ifstream artifactFile;
        artifactFile.open(copiedArtifactPath, std::ios::binary | std::ios::ate);
        if (artifactFile.fail())
        {
            return std::format("Artifact at path {} is missing. Source metadata: {}", copiedArtifactPath.string(), asset.m_SourceAssetPath.string());
        }

        // ReSharper disable once CppRedundantCastExpression
        const uint32_t uiArtifactLength = static_cast<uint32_t>(artifactFile.tellg());

        // ReSharper disable once CppRedundantCastExpression
        artifactFile.seekg((std::streamoff)0, std::ios::beg);

        if (uiArtifactLength == 0)
        {
            return std::format("{} was zero sized", copiedArtifactPath.string());
        }

        std::unique_ptr<uint8_t[]> pArtifactData(new uint8_t[uiArtifactLength]);
        artifactFile.read(reinterpret_cast<char*>(pArtifactData.get()), uiArtifactLength);

        outputFiles.emplace_back(std::move(pArtifactData), uiArtifactLength, GetAssetRelativeOutputPath(asset));
    }

    return outputFiles;
}
