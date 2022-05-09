#include "ModelAssetHandler.h"

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
            struct Binding
            {
                std::string m_Sampler;
                std::filesystem::path m_Texture;
            };

            std::filesystem::path m_Program;
            std::vector<Binding> m_Bindings;
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
            materialMetadata.m_Program = materialMetadata.m_Program.lexically_normal();

            const auto textures = material->getArray("texture");
            if (textures)
            {
                for(int i = 0; i < textures->size(); ++i)
                {
                    const auto texture = textures->getTable(i);

                    auto texturePath = relativePath.parent_path() / texture->getString("texture").second;
                    texturePath = texturePath.lexically_normal();

                    materialMetadata.m_Bindings.push_back({
                        texture->getString("sampler").second,
                        texturePath
                    });
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
        textureCount = metadata.m_Material->m_Bindings.size();
    }

    asset_builder::utility::file_utility::StructuredFileWriter writer;
    writer.AddDoubleSizedFixup("mesh");
    writer.AddDoubleSizedFixup("program");
    writer.AddData(textureCount);
    for(int i = 0; i < textureCount; i++)
    {
        writer.AddDoubleSizedFixup(std::format("sampler_{}", i));
        writer.AddDoubleSizedFixup(std::format("texture_{}", i));
    }

    writer.AddKeyedData("mesh", asset_builder::actions::getAssetRelativeName(metadata.m_Mesh));
    if (metadata.m_Material.has_value())
    {
        writer.AddKeyedData("program", asset_builder::actions::getAssetRelativeName(metadata.m_Material.value().m_Program));
        for(int i = 0; i < textureCount; i++)
        {
            writer.AddKeyedData(std::format("sampler_{}", i), metadata.m_Material.value().m_Bindings[i].m_Sampler);
            writer.AddKeyedData(std::format("texture_{}", i), asset_builder::actions::getAssetRelativeName(metadata.m_Material.value().m_Bindings[i].m_Texture));
        }
    }

    auto data = writer.GetFinalData();

    std::vector<OutputArtifact> outputs;
    outputs.emplace_back(std::move(data), writer.GetDataSize(), GetAssetRelativeOutputPath(asset));

    return outputs;
}
