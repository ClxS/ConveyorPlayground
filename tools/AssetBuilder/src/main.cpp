#include <iostream>

#include "Arguments.h"
#include "ExitCodes.h"
#include "ToolsCore/Hashing.h"

#include <filesystem>
#include <iostream>
#include <stack>

int doSpecGenerate(const Arguments&);

int main(const int argc, char **argv)
{
    Arguments args;
    if (!args.TryRead(argc, argv))
    {
        std::cerr << "Failed to read arguments";
        return static_cast<int>(ExitCode::ArgumentParseError);
    }

    using cppconv::tools::hashing::fnv1;

    switch (fnv1(args.m_Mode.m_Value))
    {
        case fnv1("generate-spec"): return doSpecGenerate(args);
        default:
            std::cerr << "Unknown verb " << args.m_Mode.m_Value << "\n";
            return static_cast<int>(ExitCode::UnknownMode);
    }
}

class AssetTree
{
public:
    class TreeNode
    {
    public:
        struct Asset
        {
            std::filesystem::path m_FullPath;
            std::filesystem::path m_RelativePath;
        };

        explicit TreeNode(std::filesystem::path rootPath)
            : m_RootPath{std::move(rootPath)}
        {
        }

        [[nodiscard]] TreeNode* GetOrCreateChildNode(const std::filesystem::path& path)
        {
            const std::filesystem::path relativePath = std::filesystem::relative(path, m_RootPath);
            for(const auto& child : m_ChildNodes)
            {
                if (child->m_RootPath == relativePath)
                {
                    return child.get();
                }
            }

            m_ChildNodes.push_back(std::make_unique<TreeNode>(relativePath));
            return m_ChildNodes.back().get();
        }

        [[nodiscard]] bool Exists(const std::filesystem::path& path) const
        {
            const std::filesystem::path relativePath = std::filesystem::relative(path, m_RootPath);
            if (std::ranges::any_of(m_ChildNodes, [relativePath](const std::unique_ptr<TreeNode>& asset) { return asset->m_RootPath == relativePath; }))
            {
                return true;
            }

            if (std::ranges::any_of(m_Assets, [relativePath](const Asset& asset) { return asset.m_FullPath == relativePath; }))
            {
                return true;
            }

            return false;
        }

        void AddAsset(const std::filesystem::path& fullPath, const std::filesystem::path& relative)
        {
            m_Assets.push_back({ fullPath, relative });
        }

        std::filesystem::path m_RootPath;

        std::vector<std::unique_ptr<TreeNode>> m_ChildNodes;
        std::vector<Asset> m_Assets;
    };

    explicit AssetTree(std::filesystem::path rootPath)
        : m_Root(std::move(rootPath))
    {

    }

    [[nodiscard]] TreeNode& GetRoot() { return m_Root; }
    [[nodiscard]] const TreeNode& GetRoot() const { return m_Root; }

private:
    TreeNode m_Root;
};

int doSpecGenerate(const Arguments& args)
{
    // Walks through and builds an asset tree in priority order, order is platform > common, private > non-private.
    namespace fs = std::filesystem;

    const fs::path dataRoot = args.m_DataRoot.m_Value;

    const fs::path commonFolder = dataRoot / "common";
    const fs::path commonPrivateFolder = dataRoot / "commonPrivate";
    const fs::path platformFolder = dataRoot / (args.m_Platform.m_Value);
    const fs::path platformPrivateFolder = dataRoot / (args.m_Platform.m_Value + "Private");

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
            std::cout << entry.path() << std::endl;
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
                    fs::path rootRelativePath = relative(entry, dataRoot);
                    node.AddAsset(rootRelativePath, relativePath);
                }
            }
        }
    }

    return 0;
}

