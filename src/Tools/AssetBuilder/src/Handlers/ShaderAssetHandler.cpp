#include "ShaderAssetHandler.h"

#include <cassert>

#include "Asset.h"

#include "FileUtility.h"
#include "ToolsCore/Hashing.h"
#include "ToolsCore/Utility/TemporaryFile.h"
#include <Windows.h>
#include <corecrt_io.h>

#include "PathUtility.h"
#include "ProcessUtility.h"

namespace
{
    struct ShaderMetadata
    {
        enum class ShaderType
        {
            Vertex,
            Fragment
        };

        ShaderType m_Type = ShaderType::Vertex;
        std::string m_Model;
        std::string m_SourceFile;
    };
}

std::variant<ShaderMetadata, ErrorString> readMetadata(const std::filesystem::path& path)
{
    using cppconv::tools::hashing::fnv1;

    const auto result = asset_builder::utility::file_utility::readTomlFromFile(path);
    if (std::holds_alternative<std::string>(result))
    {
        return std::format("Failed to read shader metadata: {} - {}\n", std::get<1>(result), path.string());
    }

    const auto table = std::get<0>(result);
    const auto keys = table->keys();
    const auto shaderInfo = table->getTable("shader");

    ShaderMetadata metadata;
    metadata.m_Model = shaderInfo->getString("model").second;
    metadata.m_SourceFile = (path.parent_path() / shaderInfo->getString("source").second).string();

    switch(fnv1(shaderInfo->getString("type").second))
    {
        case fnv1("vertex"):
            metadata.m_Type = ShaderMetadata::ShaderType::Vertex;
            break;
        case fnv1("fragment"):
            metadata.m_Type = ShaderMetadata::ShaderType::Fragment;
            break;
        default:
            return std::format("Unknown shader type ", shaderInfo->getString("type").second);
    }


    return metadata;
}

std::filesystem::path ShaderAssetHandler::GetAssetRelativeOutputPath(const Asset& fullPath)
{
    auto outputPath = fullPath.m_RelativePath;
    return outputPath.replace_extension("nshader");
}

std::variant<std::vector<OutputArtifact>, ErrorString> ShaderAssetHandler::Cook(const Asset& asset)
{
    auto metadataResult = readMetadata(asset.m_SourceAssetPath);
    if (std::holds_alternative<ErrorString>(metadataResult))
    {
        return std::get<1>(metadataResult);
    }

    auto metadata = std::get<0>(metadataResult);

    cpp_conv::tools::utility::TemporaryFile tempBinFile;

    std::string sourceFile = metadata.m_SourceFile;
    std::string outputFile = tempBinFile.GetFile().string();
    std::string platform = "windows";
    std::string shaderType;
    std::string shaderModelPrefix = "vs";
    std::string shaderModel = metadata.m_Model;

    switch(metadata.m_Type)
    {
    case ShaderMetadata::ShaderType::Vertex:
        shaderType = "vertex";
        shaderModelPrefix = "vs";
        break;
    case ShaderMetadata::ShaderType::Fragment:
        shaderType = "fragment";
        shaderModelPrefix = "ps";
        break;
    }

    auto currentDirectory = asset_builder::utility::path_utility::getCurrentPath();
    if (!currentDirectory.has_value())
    {
        return "Could not determine current directory";
    }

    std::string processName = (currentDirectory.value() / "shaderc.exe").string();
    std::string args = std::format(
        R"(-f "{}" -o "{}" --platform {} --type {} --verbose --debug -i -p {}_{})",
        sourceFile,
        outputFile,
        platform,
        shaderType,
        shaderModelPrefix,
        shaderModel);

    std::string stdOut, stdErr;
    int exitCode = asset_builder::utility::process_utility::execute(processName, args, stdOut, stdErr);

    // TODO Capture output
    if (exitCode != 0)
    {
        return std::format("shaderc failed with code {}. {} {}", exitCode, processName, args);
    }

    auto [fileContents, size] = asset_builder::utility::file_utility::readFile(outputFile);
    if (!fileContents)
    {
        return {};
    }

    std::vector<OutputArtifact> artifacts;
    artifacts.emplace_back(std::move(fileContents), size, GetAssetRelativeOutputPath(asset));

    return artifacts;
}
