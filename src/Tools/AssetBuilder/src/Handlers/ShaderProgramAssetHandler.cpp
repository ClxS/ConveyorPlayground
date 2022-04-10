#include "ShaderProgramAssetHandler.h"

#include <cassert>

#include "Asset.h"
#include "FileUtility.h"
#include "GenerateSpec.h"
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
    };

    struct Header
    {
        struct Chunk
        {
            uint32_t m_Start;
            uint32_t m_End;
        };

        Chunk m_VertexShader;
        Chunk m_FragmentShader;
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
        metadata.m_Vertex = (relativePath.parent_path() / shaderInfo->getString("vertex").second).string();
        metadata.m_Fragment = (relativePath.parent_path() / shaderInfo->getString("fragment").second).string();

        return metadata;
    }

    bool write(uint8_t*& destination, const void* source, const size_t sourceSize, size_t& destinationSizeRemaining)
    {
        if (destinationSizeRemaining < sourceSize)
        {
            return false;
        }

        std::memcpy(destination, source, sourceSize);
        destination += sourceSize;
        destinationSizeRemaining -= sourceSize;
        return true;
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

    const Header header =
    {
        {
            sizeof(Header),
            sizeof(Header) + shaderKeys[0].size() - 1
        },
        {
            sizeof(Header) + shaderKeys[0].size(),
            sizeof(Header) + shaderKeys[0].size() + shaderKeys[1].size() - 1
        },
    };

    const size_t dataSize = sizeof(Header) + shaderKeys[0].size() + shaderKeys[1].size();

    std::unique_ptr<uint8_t[]> data(new uint8_t[dataSize]);
    auto dataPtr = data.get();

    size_t bufferSizeRemaining = dataSize;
    assert(write(dataPtr, &header, sizeof(Header), bufferSizeRemaining));
    assert(write(dataPtr, shaderKeys[0].data(), shaderKeys[0].size(), bufferSizeRemaining));
    assert(write(dataPtr, shaderKeys[1].data(), shaderKeys[1].size(), bufferSizeRemaining));
    assert(bufferSizeRemaining == 0);
    std::vector<OutputArtifact> outputs;
    outputs.emplace_back(std::move(data), dataSize, GetAssetRelativeOutputPath(asset));

    return outputs;
}
