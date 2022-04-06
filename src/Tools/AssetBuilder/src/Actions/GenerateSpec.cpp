#include "GenerateSpec.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <Windows.h>

#include "Arguments.h"
#include "Asset.h"
#include "AssetHandler.h"
#include "AssetTree.h"

namespace
{
    bool doesFileContentsMatch(const std::filesystem::path& path, std::string contents)
    {
        if (std::filesystem::exists(path))
        {
            std::ifstream existingFile(path, std::ios::in | std::ios::binary);
            if (existingFile.is_open())
            {
                std::string outputStr((std::istreambuf_iterator(existingFile)), std::istreambuf_iterator<char>());
                existingFile.close();

                std::erase_if(outputStr, [](const char ch) { return ch == '\r' || ch == '\n'; });
                std::erase_if(contents, [](const char ch) { return ch == '\r' || ch == '\n'; });
                if (outputStr == contents)
                {
                    return true;
                }
            }
        }

        return false;
    }

    std::string replaceAll(std::string str, const std::string& from, const std::string& to)
    {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }

        return str;
    }

    std::string sanitizeAssetNameForVariable(const std::filesystem::path& path)
    {
        std::string filename = path.filename().string();

        const size_t dot = filename.find_last_of('.');
        if (dot == std::string::npos)
        {
            return filename;
        }

        return filename.substr(0, dot);
    }

    void writeGroup(std::stringstream& outStream, const AssetTree::TreeNode& node, const int depth, int& index)
    {
        std::string groupNamespace = node.m_Path.string();
        std::ranges::transform(groupNamespace, groupNamespace.begin(), [](const unsigned char c){ return std::tolower(c); });

        outStream << std::string(depth * 4, ' ') << "namespace " << groupNamespace << " {\n";

        for(auto& group : node.m_ChildNodes)
        {
            writeGroup(outStream, *group, depth + 1, index);
        }

        for(const auto& [fullPath, relativePath, _] : node.m_Assets)
        {
            outStream
                << std::string((depth + 1) * 4, ' ')
                << "inline constexpr RegistryId c_"
                << sanitizeAssetNameForVariable(relativePath)
                << " = " << index << ";\n";
            index++;
        }

        if (!node.m_Assets.empty())
        {
            outStream << std::string((depth + 1) * 4, ' ') << "inline constexpr std::array<RegistryId, " << node.m_Assets.size() << "> c_AllAssets {{ \n";
            for(const auto& [fullPath, relativePath, _] : node.m_Assets)
            {
                outStream << std::string((depth + 2) * 4, ' ') << "c_" << sanitizeAssetNameForVariable(relativePath) << ",\n";
            }
            outStream<< std::string((depth + 1) * 4, ' ') << "}};\n";
        }

        outStream << std::string(depth * 4, ' ') << "}\n";
    }

    std::string getAssetRelativeName(std::filesystem::path relativePath)
    {
        relativePath = relativePath.replace_extension();
        std::string namespaceName = relativePath.string();
        namespaceName = replaceAll(namespaceName, "\\", "::");
        namespaceName = replaceAll(namespaceName, "/", "::");
        return namespaceName;
    }

    void writeAssets(std::stringstream& outStream, const AssetTree::TreeNode& node, const int depth, int& index)
    {
        for(auto& group : node.m_ChildNodes)
        {
            writeAssets(outStream, *group, depth, index);
        }

        for(const auto& asset : node.m_Assets)
        {
            assert(asset.m_pAssociatedHandler);
            outStream << std::string((depth + 1) * 4, ' ') << "{ std::filesystem::path(R\"("
                << asset.m_pAssociatedHandler->GetAssetRelativeOutputPath(asset).string()
                << ")\"),"
                << "\"" << getAssetRelativeName(asset.m_RelativePath) << "\""
                << "},\n";
        }
    }

    std::string generateFileFromTree(const AssetTree& tree, const std::string& fileNamespace)
    {
        std::stringstream outFile;
        outFile << "#pragma once\n";
        outFile << "#include <array>\n";
        outFile << "#include <filesystem>\n";
        outFile << "#include <string>\n";
        outFile << "namespace " << fileNamespace << " {\n";
        outFile << std::string(1 * 4, ' ') << "struct RegistryId\n";
        outFile << std::string(1 * 4, ' ') << "{\n";
        outFile << std::string(2 * 4, ' ') << "constexpr RegistryId(uint32_t value) : m_Value(value) {}\n";
        outFile << std::string(2 * 4, ' ') << "static RegistryId Invalid() { return 0xFFFFFFFF; }\n";
        outFile << std::string(2 * 4, ' ') << "bool operator<(const RegistryId& other) const { return m_Value < other.m_Value; }\n";
        outFile << std::string(2 * 4, ' ') << "uint32_t m_Value;\n";
        outFile << std::string(1 * 4, ' ') << "};\n";
        outFile << std::string(1 * 4, ' ') << "struct Asset { std::filesystem::path m_Path; std::string m_RelativeName; };\n";


        int index = 0;
        for(auto& group : tree.GetRoot().m_ChildNodes)
        {
            writeGroup(outFile, *group, 1, index);
        }

        outFile << std::string(1 * 4, ' ') << "const std::array<Asset, " << index + 1 << "> c_Files = {{\n";
        for(auto& group : tree.GetRoot().m_ChildNodes)
        {
            writeAssets(outFile, *group, 1, index);
        }
        outFile << std::string(1 * 4, ' ') << "}};\n";

        outFile << std::string(1 * 4, ' ') << "inline bool tryLookUpId(const std::string_view& str, RegistryId* outId) {\n";
        outFile << std::string(2 * 4, ' ') << "if (!outId) return false;\n";
        outFile << std::string(2 * 4, ' ') << "for(uint32_t i = 0; i < c_Files.size(); i++) {\n";
        outFile << std::string(3 * 4, ' ') << "if (c_Files[i].m_RelativeName == str) { *outId = i; return true; }\n";
        outFile << std::string(2 * 4, ' ') << "}\n";
        outFile << std::string(2 * 4, ' ') << "return false;\n";
        outFile << std::string(1 * 4, ' ') << "}\n";

        outFile << "}";
        return outFile.str();
    }
}

ExitCode asset_builder::actions::generateSpec(const Arguments& args)
{
    if (!args.m_OutputFile.m_bIsSet)
    {
        return ExitCode::MissingRequiredArgument;
    }

    namespace fs = std::filesystem;

    const AssetTree tree = AssetTree::CreateFromFileStructure(args.m_DataRoot.m_Value, args.m_Platform.m_Value);
    const std::string outputFileText = generateFileFromTree(tree, args.m_Namespace.m_Value);

    const fs::path outputPath = args.m_OutputFile.m_Value;
    if (doesFileContentsMatch(outputPath, outputFileText))
    {
        printf("File is up to date");
        return ExitCode::Success;
    }

    fs::create_directory(outputPath.parent_path());
    std::ofstream out(outputPath);
    out << outputFileText;
    out.close();

    if (!doesFileContentsMatch(outputPath, outputFileText))
    {
        printf("File did not match after writing");
        return ExitCode::FileOutputMismatch;
    }

    return ExitCode::Success;
}
