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
        struct TextureSlotInfo
        {
            uint8_t m_Slot{};
            bool m_Reserved{};
            std::string m_Type{};
        };

        std::filesystem::path m_Vertex;
        std::filesystem::path m_Fragment;
        int8_t m_TextureSlotCount{};
        std::vector<TextureSlotInfo> m_TextureSlotInfo;
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
        metadata.m_TextureSlotCount = static_cast<uint8_t>(shaderInfo->getInt("textureSlotCount").second);

        const auto infoArray = shaderInfo->getArray("textureSlotInfo");
        if (infoArray)
        {
            for(int i = 0; i < infoArray->size(); ++i)
            {
                const auto infoTable = infoArray->getTable(i);
                const auto [hasSlot, slot] = infoTable->getInt("slot");
                if (!hasSlot)
                {
                    return "ShaderProgram textureSlotInfo is missing required 'slot' value";
                }

                ShaderProgramMetadata::TextureSlotInfo info{};
                info.m_Slot = static_cast<uint8_t>(slot);

                const auto [hasReservedValue, isReserved] = infoTable->getBool("reserved");
                const auto [hasType, type] = infoTable->getString("type");
                if (hasReservedValue && !hasType)
                {
                    return "textureSlotInfo has a reserved entry with no 'type' value. This is not allowed";
                }

                info.m_Reserved = hasReservedValue && isReserved;
                if (hasType)
                {
                    info.m_Type = type;
                }

                metadata.m_TextureSlotInfo.push_back(info);
            }
        }

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

    // Header
    writer.AddDoubleSizedFixup("vertex");
    writer.AddDoubleSizedFixup("fragment");
    writer.AddData<int32_t>(metadata.m_TextureSlotCount);
    writer.AddData<int32_t>(metadata.m_TextureSlotInfo.size());
    writer.AddDoubleSizedFixup("textureData");

    // Payload
    writer.AddKeyedData("vertex", shaderKeys[0].data(), shaderKeys[0].size());
    writer.AddKeyedData("fragment", shaderKeys[1].data(), shaderKeys[1].size());
    if (!metadata.m_TextureSlotInfo.empty())
    {
        writer.SetKeyedDataRangeFromCurrent("textureData", sizeof(uint8_t) * 4 + sizeof(uint32_t) * 2);
        for(const auto& slot : metadata.m_TextureSlotInfo)
        {
            writer.AddData<uint8_t>(slot.m_Slot);
            writer.AddData<uint8_t>(static_cast<uint8_t>(slot.m_Reserved ? 1 : 0));
            writer.AddPadding(sizeof(uint8_t) * 2); // Padding
            writer.AddDoubleSizedFixup("textureType_" + std::to_string(slot.m_Slot));
        }
    }

    for(const auto& slot : metadata.m_TextureSlotInfo)
    {
        writer.AddKeyedData("textureType_" + std::to_string(slot.m_Slot), slot.m_Type.data(), slot.m_Type.size());
    }

    auto data = writer.GetFinalData();

    std::vector<OutputArtifact> outputs;
    outputs.emplace_back(std::move(data), writer.GetDataSize(), GetAssetRelativeOutputPath(asset));

    return outputs;
}
