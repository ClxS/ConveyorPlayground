#include "ModelAssetHandler.h"

#include <cassert>
#include <numeric>

#include "Asset.h"
#include "FileUtility.h"
#include "GenerateSpec.h"
#include "StructuredFileWriter.h"
#include "ToolsCore/Hashing.h"

namespace
{
    struct ModelMetadata
    {
        struct Material
        {
            std::filesystem::path m_Program;
            std::vector<std::filesystem::path> m_Textures;
        };

        std::filesystem::path m_Mesh;
        std::optional<Material> m_Material;
    };

    std::variant<ModelMetadata, ErrorString> readMetadata(const std::filesystem::path& path, const std::filesystem::path& relativePath)
    {
        using cppconv::tools::hashing::fnv1;

        const auto result = asset_builder::utility::file_utility::readTomlFromFile(path);
        if (std::holds_alternative<std::string>(result))
        {
            return std::format("Failed to read shader metadata: {} - {}\n", std::get<1>(result), path.string());
        }

        const auto table = std::get<0>(result);
        const auto keys = table->keys();
        const auto shaderInfo = table->getTable("model");

        ModelMetadata metadata;
        metadata.m_Mesh = relativePath.parent_path() / shaderInfo->getString("mesh").second;

        const auto material = shaderInfo->getTable("material");
        if (material)
        {
            ModelMetadata::Material materialMetadata;
            materialMetadata.m_Program = relativePath.parent_path() / material->getString("program").second;
            const auto textures = material->getArray("textures");
            if (textures)
            {
                for(int i = 0; i < textures->size(); ++i)
                {
                    materialMetadata.m_Textures.push_back(relativePath.parent_path() / textures->getString(0).second);
                }
            }

            metadata.m_Material = materialMetadata;
        }


        return metadata;
    }
}

std::filesystem::path ModelAssetHandler::GetAssetRelativeOutputPath(const Asset& fullPath)
{
    auto outputPath = fullPath.m_RelativePath;
    return outputPath.replace_extension("nmodel");
}

std::variant<std::vector<OutputArtifact>, ErrorString> ModelAssetHandler::Cook(const Asset& asset)
{
    auto metadataResult = readMetadata(asset.m_SourceAssetPath, asset.m_RelativePath);
    if (std::holds_alternative<ErrorString>(metadataResult))
    {
        return std::get<1>(metadataResult);
    }

    const auto metadata = std::get<0>(metadataResult);

    int64_t textureCount = 0;
    if (metadata.m_Material.has_value())
    {
        textureCount = metadata.m_Material->m_Textures.size();
    }

    asset_builder::utility::file_utility::StructuredFileWriter writer;
    writer.AddDoubleSizedFixup("mesh");
    writer.AddDoubleSizedFixup("program");
    writer.AddData(textureCount);
    for(int i = 0; i < textureCount; i++)
    {
        writer.AddDoubleSizedFixup(std::format("texture_{}", i));
    }

    writer.AddKeyedData("mesh", asset_builder::actions::getAssetRelativeName(metadata.m_Mesh));
    if (metadata.m_Material.has_value())
    {
        writer.AddKeyedData("program", asset_builder::actions::getAssetRelativeName(metadata.m_Material.value().m_Program));
        for(int i = 0; i < textureCount; i++)
        {
            writer.AddKeyedData(std::format("texture_{}", i), asset_builder::actions::getAssetRelativeName(metadata.m_Material.value().m_Textures[i]));
        }
    }

    auto data = writer.GetFinalData();

    std::vector<OutputArtifact> outputs;
    outputs.emplace_back(std::move(data), writer.GetDataSize(), GetAssetRelativeOutputPath(asset));

    return outputs;
}
