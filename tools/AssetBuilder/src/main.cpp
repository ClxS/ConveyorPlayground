#include <iostream>

#include "Arguments.h"
#include "ExitCodes.h"
#include "ToolsCore/Hashing.h"

#include <filesystem>
#include <iostream>
#include <stack>
#include <fstream>

ExitCode doSpecGenerate(const Arguments&);
ExitCode mainImpl(int argc, char **argv);

int main(const int argc, char **argv)
{
    return static_cast<int>(mainImpl(argc, argv));
}

ExitCode mainImpl(const int argc, char **argv)
{
    Arguments args;
    if (!args.TryRead(argc, argv))
    {
        std::cerr << "Failed to read arguments";
        return ExitCode::ArgumentParseError;
    }

    using cppconv::tools::hashing::fnv1;

    switch (fnv1(args.m_Mode.m_Value))
    {
        case fnv1("generate-spec"): return doSpecGenerate(args);
        default:
            std::cerr << "Unknown verb " << args.m_Mode.m_Value << "\n";
            return ExitCode::UnknownMode;
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
            : m_Path{std::move(rootPath)}
        {
        }

        [[nodiscard]] TreeNode* GetOrCreateChildNode(const std::filesystem::path& path)
        {
            const std::filesystem::path relativePath = std::filesystem::relative(path, m_Path);
            for(const auto& child : m_ChildNodes)
            {
                if (child->m_Path == relativePath)
                {
                    return child.get();
                }
            }

            m_ChildNodes.push_back(std::make_unique<TreeNode>(relativePath));
            return m_ChildNodes.back().get();
        }

        [[nodiscard]] bool Exists(const std::filesystem::path& path) const
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

        void AddAsset(const std::filesystem::path& fullPath, const std::filesystem::path& relative)
        {
            m_Assets.push_back({ fullPath, relative });
        }

        std::filesystem::path m_Path;

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

AssetTree generateAssetTree(const std::string& root, const std::string& platform)
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

    return tree;
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

void writeGroup(std::stringstream& outStream, const AssetTree::TreeNode& node, const int depth)
{
    std::string groupNamespace = node.m_Path.string();
    std::ranges::transform(groupNamespace, groupNamespace.begin(), [](const unsigned char c){ return std::tolower(c); });

    outStream << std::string(depth * 4, ' ') << "namespace " << groupNamespace << " {\n";

    for(auto& group : node.m_ChildNodes)
    {
        writeGroup(outStream, *group, depth + 1);
    }

    for(const auto& [fullPath, relativePath] : node.m_Assets)
    {
        outStream
            << std::string((depth + 1) * 4, ' ')
            << "const std::filesystem::path c_"
            << sanitizeAssetNameForVariable(relativePath)
            << " = R\"("
            << relativePath.string()
            << ")\";\n";
    }

    outStream << std::string((depth + 1) * 4, ' ') << "inline const std::array<const std::filesystem::path*, " << node.m_Assets.size() << ">& getAll() {\n";
    outStream << std::string((depth + 2) * 4, ' ') << "static std::array<const std::filesystem::path*, " << node.m_Assets.size() << "> s_all { \n";
    for(const auto& [fullPath, relativePath] : node.m_Assets)
    {
        outStream << std::string((depth + 3) * 4, ' ') << "&c_" << sanitizeAssetNameForVariable(relativePath) << ",\n";
    }
    outStream<< std::string((depth + 2) * 4, ' ') << "};\n";
    outStream << std::string((depth + 2) * 4, ' ') << "return s_all;\n";
    outStream<< std::string((depth + 1) * 4, ' ') << "}\n";

    outStream << std::string(depth * 4, ' ') << "}\n";
}

std::string generateFileFromTree(const AssetTree& tree, const std::string& fileNamespace)
{
    std::stringstream outFile;
    outFile << "#include <filesystem>\n";
    outFile << "namespace " << fileNamespace << " {\n";

    for(auto& group : tree.GetRoot().m_ChildNodes)
    {
        writeGroup(outFile, *group, 1);
    }

    outFile << "}";
    return outFile.str();
}

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

ExitCode doSpecGenerate(const Arguments& args)
{
    if (!args.m_OutputFile.m_bIsSet)
    {
        return ExitCode::MissingRequiredArgument;
    }

    namespace fs = std::filesystem;

    const AssetTree tree = generateAssetTree(args.m_DataRoot.m_Value, args.m_Platform.m_Value);
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

