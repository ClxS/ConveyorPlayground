#include "MeshAssetHandler.h"

#include <cassert>

#include "Asset.h"

#include "FileUtility.h"
#include "ToolsCore/Hashing.h"
#include "ToolsCore/Utility/TemporaryFile.h"
#include <Windows.h>

#include "PathUtility.h"
#include "ProcessUtility.h"

namespace
{
    struct MeshMetadata
    {
        std::string m_SourceFile;
        double m_Scale;
    };
}

std::variant<MeshMetadata, ErrorString> readMetadata(const std::filesystem::path& path)
{
    using cppconv::tools::hashing::fnv1;

    const auto result = asset_builder::utility::file_utility::readTomlFromFile(path);
    if (std::holds_alternative<std::string>(result))
    {
        return std::format("Failed to read shader metadata: {} - {}\n", std::get<1>(result), path.string());
    }

    const auto table = std::get<0>(result);
    const auto keys = table->keys();
    const auto shaderInfo = table->getTable("mesh");

    MeshMetadata metadata;
    metadata.m_SourceFile = (path.parent_path() / shaderInfo->getString("source").second).string();

    auto [hasScale, scale] = shaderInfo->getDouble("scale");
    if (hasScale)
    {
        metadata.m_Scale = scale;
    }

    return metadata;
}

std::filesystem::path MeshAssetHandler::GetAssetRelativeOutputPath(const Asset& fullPath)
{
    auto outputPath = fullPath.m_RelativePath;
    return outputPath.replace_extension("nmesh");
}

std::variant<std::vector<OutputArtifact>, ErrorString> MeshAssetHandler::Cook(const Asset& asset)
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

    auto currentDirectory = asset_builder::utility::path_utility::getCurrentPath();
    if (!currentDirectory.has_value())
    {
        return "Could not determine current directory";
    }

    std::string processName = (currentDirectory.value() / "geometryc.exe").string();
    std::string args = std::format(
        R"(-f "{}" -o "{}" -s {} --flipv --ccw)",
        sourceFile,
        outputFile,
        metadata.m_Scale);

    std::string stdOut, stdErr;
    int exitCode = asset_builder::utility::process_utility::execute(processName, args, stdOut, stdErr);

    // TODO Capture output
    if (exitCode != 0)
    {
        return std::format("geometryc failed with code {}. {} {}", exitCode, processName, args);
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
