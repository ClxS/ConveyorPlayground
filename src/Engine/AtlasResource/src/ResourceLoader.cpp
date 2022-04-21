#include "AtlasResourcePCH.h"
#include "ResourceLoader.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <mutex>
#include <cstdlib>

namespace
{
    atlas::resource::FileData getFileData(const atlas::resource::RegistryId id, const std::filesystem::path& path)
    {
        const auto rootPath = std::filesystem::current_path();
        const auto filePath = rootPath
            /
            "data" /
            path;

        std::ifstream file;
        file.open(filePath, std::ios::binary | std::ios::ate);
        if (file.fail())
        {
            return {filePath, atlas::resource::RegistryId::Invalid(), nullptr, 0};
        }

        // ReSharper disable once CppRedundantCastExpression
        const uint32_t uiLength = static_cast<uint32_t>(file.tellg());

        // ReSharper disable once CppRedundantCastExpression
        file.seekg((std::streamoff)0, std::ios::beg);

        if (uiLength == 0)
        {
            return {filePath, atlas::resource::RegistryId::Invalid(), nullptr, 0};
        }

        std::unique_ptr<uint8_t[]> pData(new uint8_t[uiLength]);
        file.read(reinterpret_cast<char*>(pData.get()), uiLength);

        return {filePath, id, std::move(pData), uiLength};
    }
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> atlas::resource::ResourceLoader::LoadAsset(
    const int32_t typeId,
    const int32_t bundleId,
    const RegistryId registryId,
    const bool uncached)
{
    assert(bundleId < ms_Bundles.size());
    assert(typeId < ms_TypeHandlers.size());
    assert(ms_TypeHandlers[typeId]);

    auto& bundle = ms_Bundles[bundleId];
    assert(bundle.m_Bundle);

    std::lock_guard lock(ms_StateMutex);
    if(!uncached)
    {
        auto& cacheEntry = bundle.m_BundleCache.m_Entries[registryId.m_Value];
        if (cacheEntry.m_Asset)
        {
            cacheEntry.m_LastAccess = std::chrono::steady_clock::now();
            return cacheEntry.m_Asset;
        }
    }

    const auto entry = bundle.m_Bundle->GetAsset(registryId);
    if (!entry.has_value())
    {
        std::cerr << std::format("Failed to get asset with id ({}:{})", bundleId, registryId.m_Value);
        return nullptr;
    }

    FileData fileData = getFileData(registryId, entry.value().m_Path);
    if (!fileData.m_pData)
    {
        return nullptr;
    }

    auto asset = ms_TypeHandlers[typeId](fileData);
    if (uncached)
    {
        return asset;
    }

    if (!asset)
    {
        return nullptr;
    }

    auto& cacheEntry = bundle.m_BundleCache.m_Entries[registryId.m_Value];
    cacheEntry.m_Asset = asset;
    cacheEntry.m_LastAccess = std::chrono::steady_clock::now();
    return asset;
}
