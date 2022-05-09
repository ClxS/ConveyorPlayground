#include "AssetProcessor.h"

#include <iostream>
#include <unordered_map>

#include "DataAssetHandler.h"
#include "MapAssetHandler.h"
#include "ModelAssetHandler.h"
#include "ShaderAssetHandler.h"
#include "Bgfx/MeshAssetHandler.h"
#include "Bgfx/Texture2dAsssetHandler.h"
#include "ShaderProgramAssetHandler.h"
#include "Rml/RmlDataHandler.h"

std::unordered_map<std::string, std::unique_ptr<AssetHandler>> s_handlers;

void AssetProcessor::initDefaults()
{
    registerHandler<DataAssetHandler>();
    registerHandler<MapAssetHandler>();
    registerHandler<ShaderAssetHandler>();
    registerHandler<ShaderProgramAssetHandler>();
    registerHandler<Texture2DAssetHandler>();
    registerHandler<MeshAssetHandler>();
    registerHandler<ModelAssetHandler>();
    registerHandler<RmlPageDataHandler>();
    registerHandler<RmlFontDataHandler>();
    registerHandler<RmlCssDataHandler>();
}

void AssetProcessor::registerHandler(const std::string& key, std::unique_ptr<AssetHandler>&& pHandler)
{
    s_handlers[key] = std::move(pHandler);
}

std::tuple<AssetHandler*, std::string> AssetProcessor::getAssetFileHandler(const std::filesystem::path& path)
{
    std::ifstream file;
    file.open(path, std::ios::binary | std::ios::ate);
    if (file.fail())
    {
        return { nullptr, "" };
    }

    // ReSharper disable once CppRedundantCastExpression
    const uint32_t uiLength = static_cast<uint32_t>(file.tellg());

    // ReSharper disable once CppRedundantCastExpression
    file.seekg((std::streamoff)0, std::ios::beg);

    if (uiLength == 0)
    {
        return { nullptr, "" };
    }

    const std::unique_ptr<uint8_t[]> pData(new uint8_t[uiLength]);
    file.read(reinterpret_cast<char*>(pData.get()), uiLength);

    const auto pStrData = reinterpret_cast<const char*>(pData.get());
    const std::string input(pStrData, (int)(uiLength / sizeof(char)));
    const auto [table, errors] = toml::parse(input);
    if (!table)
    {
        std::cerr << std::format("Failed to read TOML: {} - {}\n", errors, path.string());
        return { nullptr, "" };
    }

    auto keys = table->keys();
    if (keys.size() != 1)
    {
        std::cerr << std::format("Resource metadata tables can only contain a maximum of 1 root level keys. File {} contains {}\n", path.string(), keys.size());
        return { nullptr, "" };
    }

    const auto handlerIt = s_handlers.find(keys[0]);
    if (handlerIt == s_handlers.end())
    {
        return { nullptr, "" };
    }

    return { (*handlerIt).second.get(), keys[0] };
}
