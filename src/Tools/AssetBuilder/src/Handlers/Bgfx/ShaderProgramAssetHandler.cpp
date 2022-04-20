#include "ShaderProgramAssetHandler.h"

#include <cassert>

#include "Asset.h"
#include "FileUtility.h"
#include "GenerateSpec.h"
#include "StructuredFileWriter.h"
#include "ToolsCore/Hashing.h"

namespace
{
    struct ShaderProgramMetadata
    {
        enum class ShaderType
        {
            Vertex,
            Fragment
        };

        std::filesystem::path m_Vertex;
        std::filesystem::path m_Fragment;
        int64_t m_TextureSlotCount;
    };

    std::variant<ShaderProgramMetadata, ErrorString> readMetadata(const std::filesystem::path& path, const std::filesystem::path& relativePath)
    {
        using cppconv::tools::hashing::fnv1;

        const auto result = asset_builder::utility::file_utility::readTomlFromFile(path);
        if (std::holds_alternative<std::string>(result))
        {
            return std::format("Failed to read shader metadata: {} - {}\n", std::get<1>(result), path.string());
        }

        const auto table = std::get<0>(result);
        const auto keys = table->keys();
        const auto shaderInfo = table->getTable("shaderProgram");

        ShaderProgramMetadata metadata;
        metadata.m_Vertex = (relativePath.parent_path() / shaderInfo->getString("vertex").second);
        metadata.m_Fragment = (relativePath.parent_path() / shaderInfo->getString("fragment").second);

        return metadata;
    }
}

std::filesystem::path ShaderProgramAssetHandler::GetAssetRelativeOutputPath(const Asset& fullPath)
{
    auto outputPath = fullPath.m_RelativePath;
    return outputPath.replace_extension("nprog");
}

std::variant<std::vector<OutputArtifact>, ErrorString> ShaderProgramAssetHandler::Cook(const Asset& asset)
{
    auto metadataResult = readMetadata(asset.m_SourceAssetPath, asset.m_RelativePath);
    if (std::holds_alternative<ErrorString>(metadataResult))
    {
        return std::get<1>(metadataResult);
    }

    const auto metadata = std::get<0>(metadataResult);
    const std::vector shaderKeys =
    {
        asset_builder::actions::getAssetRelativeName(metadata.m_Vertex),
        asset_builder::actions::getAssetRelativeName(metadata.m_Fragment),
    };

    asset_builder::utility::file_utility::StructuredFileWriter writer;
    writer.AddDoubleSizedFixup("vertex");
    writer.AddDoubleSizedFixup("fragment");
    writer.AddData(metadata.m_TextureSlotCount);

    writer.AddKeyedData("vertex", shaderKeys[0].data(), shaderKeys[0].size());
    writer.AddKeyedData("fragment", shaderKeys[1].data(), shaderKeys[1].size());

    auto data = writer.GetFinalData();

    std::vector<OutputArtifact> outputs;
    outputs.emplace_back(std::move(data), writer.GetDataSize(), GetAssetRelativeOutputPath(asset));

    return outputs;
}
