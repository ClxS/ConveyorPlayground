#include "Cook.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <variant>
#include <Windows.h>

#include "Arguments.h"
#include "Asset.h"
#include "AssetHandler.h"
#include "AssetTree.h"


void cookAssets(const std::vector<std::string>& types, const AssetTree::TreeNode& node, const std::filesystem::path& dataRoot, const std::filesystem::path& outputRoot)
{
    for(auto& group : node.m_ChildNodes)
    {
        cookAssets(types, *group, dataRoot, outputRoot);
    }

    for(const auto& asset : node.m_Assets)
    {
        assert(asset.m_pAssociatedHandler);

        if (!types.empty())
        {
            if (std::ranges::find(types, asset.m_Type) == types.end())
            {
                continue;
            }
        }

        std::filesystem::path outputPath = outputRoot / asset.m_pAssociatedHandler->GetAssetRelativeOutputPath(asset);
        if (!exists(outputPath.parent_path()) && !create_directories(outputPath.parent_path()))
        {
            std::cerr << "Failed to create directory " << outputPath.parent_path() << "\n";
            continue;
        }

        auto assets = asset.m_pAssociatedHandler->Cook(asset);
        if (std::holds_alternative<ErrorString>(assets))
        {
            std::cerr << std::get<1>(assets) << "\n";
            continue;
        }

        for(auto& writableAsset : std::get<0>(assets))
        {
            std::ofstream file;
            auto outputFile = outputRoot / writableAsset.m_OutputPath;
            file.open(outputFile, std::ios::binary | std::ios::out);
            if (file.fail())
            {
                std::cerr << std::format("Unable to open path for writing {}. {}\n", writableAsset.m_OutputPath.string(), GetLastError());
                continue;
            }

            file.write(reinterpret_cast<const char*>(writableAsset.m_pData.get()), writableAsset.m_DataSize);
        }

    }
}
ExitCode asset_builder::actions::cook(const Arguments& args)
{
    if (!args.m_OutputFile.m_bIsSet)
    {
        return ExitCode::MissingRequiredArgument;
    }

    namespace fs = std::filesystem;

    const fs::path dataRoot = args.m_DataRoot.m_Value;
    const fs::path outputRoot = args.m_OutputFile.m_Value;
    const AssetTree tree = AssetTree::CreateFromFileStructure(args.m_DataRoot.m_Value, args.m_Platform.m_Value);
    for(auto& group : tree.GetRoot().m_ChildNodes)
    {
        cookAssets(args.m_Types.m_Value, *group, dataRoot, outputRoot);
    }

    return ExitCode::Success;
}
