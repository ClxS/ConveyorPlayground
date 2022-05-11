#include "FileUtility.h"
#include "PathUtility.h"
#include "ProcessUtility.h"
#include "Texture2dAsssetHandler.h"
#include "ToolsCore/Utility/TemporaryFile.h"

namespace
{
    struct Metadata
    {
        std::string m_SourceFile;
        std::string m_Format = "bc2";
    };

    std::variant<Metadata, ErrorString> readMetadata(const std::filesystem::path& path)
    {
        const auto result = asset_builder::utility::file_utility::readTomlFromFile(path);
        if (std::holds_alternative<std::string>(result))
        {
            return std::format("Failed to read shader metadata: {} - {}\n", std::get<1>(result), path.string());
        }

        const auto table = std::get<0>(result);
        const auto keys = table->keys();
        const auto shaderInfo = table->getTable("texture2d");

        Metadata metadata;
        metadata.m_SourceFile = (path.parent_path() / shaderInfo->getString("texture").second).string();

        auto [hasFormat, format] = shaderInfo->getString("format");
        if (hasFormat)
        {
            metadata.m_Format = format;
        }

        return metadata;
    }
}

std::variant<std::vector<OutputArtifact>, ErrorString> Texture2DAssetHandler::Cook(const Asset& asset)
{
    auto metadataResult = readMetadata(asset.m_SourceAssetPath);
    if (std::holds_alternative<ErrorString>(metadataResult))
    {
        return std::get<1>(metadataResult);
    }

    auto metadata = std::get<0>(metadataResult);

    cpp_conv::tools::utility::TemporaryFile tempBinFile(".dds");

    std::string sourceFile = metadata.m_SourceFile;
    std::string outputFile = tempBinFile.GetFile().string();

    auto currentDirectory = asset_builder::utility::path_utility::getCurrentPath();
    if (!currentDirectory.has_value())
    {
        return "Could not determine current directory";
    }

    std::string processName = (currentDirectory.value() / "texturec.exe").string();
    std::string args = std::format(
        R"(-f "{}" -o "{}" -m {})",
        sourceFile,
        outputFile,
        metadata.m_Format);

    std::string stdOut, stdErr;
    int exitCode = asset_builder::utility::process_utility::execute(processName, args, stdOut, stdErr);

    // TODO Capture output
    if (exitCode != 0)
    {
        return std::format("texturec failed with code {}. {} {}", exitCode, processName, args);
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
