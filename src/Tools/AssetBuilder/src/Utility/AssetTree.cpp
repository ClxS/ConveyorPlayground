#include "AssetTree.h"

#include <iostream>
#include <stack>

#include "Asset.h"
#include "Processing/AssetProcessor.h"

AssetTree::TreeNode::TreeNode(std::filesystem::path rootPath, std::filesystem::path fullPath)
    : m_Path{std::move(rootPath)}
    , m_FullPath{std::move(fullPath)}
{
}

AssetTree::TreeNode* AssetTree::TreeNode::GetOrCreateChildNode(const std::filesystem::path& path)
{
    const std::filesystem::path relativePath = std::filesystem::relative(path, m_FullPath);
    for(const auto& child : m_ChildNodes)
    {
        if (child->m_Path == relativePath)
        {
            return child.get();
        }
    }

    m_ChildNodes.push_back(std::make_unique<TreeNode>(relativePath, path));
    return m_ChildNodes.back().get();
}

bool AssetTree::TreeNode::Exists(const std::filesystem::path& path) const
{
    if (std::ranges::any_of(m_ChildNodes, [path](const std::unique_ptr<TreeNode>& asset) { return asset->m_Path == path; }))
    {
        return true;
    }

    if (std::ranges::any_of(m_Assets, [path](const Asset& asset) { return asset.m_RelativePath == path; }))
    {
        return true;
    }

    return false;
}

void AssetTree::TreeNode::AddAsset(const std::filesystem::path& fullPath, const std::filesystem::path& relative,
    AssetHandler* assetHandler, const std::string& type)
{
    m_Assets.push_back({ fullPath, relative, assetHandler, type });
}

AssetTree::AssetTree(std::filesystem::path rootPath)
    : m_Root(rootPath, std::move(rootPath))
{

}

AssetTree AssetTree::CreateFromFileStructure(const std::string& root, const std::string& platform)
{
    // Walks through and builds an asset tree in priority order, order is platform > common, private > non-private.
    namespace fs = std::filesystem;

    const fs::path dataRoot = root;

    const fs::path commonFolder = dataRoot / "common";
    const fs::path commonPrivateFolder = dataRoot / "commonPrivate";
    const fs::path platformFolder = dataRoot / (platform);
    const fs::path platformPrivateFolder = dataRoot / (platform + "Private");

    AssetTree tree("");
    std::stack<std::tuple<fs::path, fs::path, AssetTree::TreeNode&>> sourceFolders;
    if (exists(commonFolder))
    {
        sourceFolders.push({commonFolder, commonFolder, tree.GetRoot()});
    }
    if (exists(commonPrivateFolder))
    {
        sourceFolders.push({commonPrivateFolder, commonPrivateFolder, tree.GetRoot()});
    }
    if (exists(platformFolder))
    {
        sourceFolders.push({platformFolder, platformFolder, tree.GetRoot()});
    }
    if (exists(platformPrivateFolder))
    {
        sourceFolders.push({platformPrivateFolder, platformPrivateFolder, tree.GetRoot()});
    }

    while(!sourceFolders.empty())
    {
        auto [ path, branchRoot, node ] = sourceFolders.top();
        sourceFolders.pop();

        for (const auto & entry : fs::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                fs::path relativePath = relative(entry, branchRoot);
                auto childNode = node.GetOrCreateChildNode(relativePath);
                sourceFolders.push({ entry, branchRoot, *childNode });
            }
            else
            {
                fs::path relativePath = relative(entry, branchRoot);
                if (!node.Exists(relativePath))
                {
                    const auto pathAsStr = relativePath.string();
                    const bool isMetadata = relativePath.has_extension() && relativePath.extension() == ".metadata";
                    if (isMetadata)
                    {
                        auto [handler, type] = AssetProcessor::getAssetFileHandler(entry);
                        if (handler)
                        {
                            node.AddAsset(entry, relativePath, handler, type);
                        }
                        else
                        {
                            std::cerr << "No handler found for " << entry << "\n";
                        }
                    }
                }
            }
        }
    }

    return tree;
}
