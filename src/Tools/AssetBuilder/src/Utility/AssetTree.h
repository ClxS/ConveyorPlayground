#pragma once
#include <filesystem>

class AssetHandler;
struct Asset;

class AssetTree
{
public:
    class TreeNode
    {
    public:
        TreeNode(std::filesystem::path relativePath, std::filesystem::path fullPath);

        [[nodiscard]] TreeNode* GetOrCreateChildNode(const std::filesystem::path& path);

        [[nodiscard]] bool Exists(const std::filesystem::path& path) const;

        void AddAsset(const std::filesystem::path& fullPath, const std::filesystem::path& relative, AssetHandler* assetHandler, const std::string& type);

        std::filesystem::path m_Path;
        std::filesystem::path m_FullPath;

        std::vector<std::unique_ptr<TreeNode>> m_ChildNodes;
        std::vector<Asset> m_Assets;
    };

    explicit AssetTree(std::filesystem::path rootPath);

    [[nodiscard]] TreeNode& GetRoot() { return m_Root; }
    [[nodiscard]] const TreeNode& GetRoot() const { return m_Root; }

    static AssetTree CreateFromFileStructure(const std::string& root, const std::string& platform);

private:
    TreeNode m_Root;
};
