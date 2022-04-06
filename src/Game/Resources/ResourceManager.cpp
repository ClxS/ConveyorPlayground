#include "ResourceManager.h"
#include "Profiler.h"
#include "ResourceAsset.h"

#include "FactoryRegistry.h"
#include "ItemRegistry.h"
#include "MapLoadHandler.h"
#include "TileLoadHandler.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <ranges>

using FileData = cpp_conv::resources::resource_manager::FileData;
using RegistryId = cpp_conv::resources::registry::RegistryId;
using SharedResourcePtr = std::shared_ptr<cpp_conv::resources::ResourceAsset>;
using TypeId = size_t;

struct LoadedAsset
{
    SharedResourcePtr m_ptr;
    std::chrono::steady_clock::time_point m_lastAccess;
};

using LoadedAssetContainer = std::map<RegistryId, LoadedAsset>;
static std::map<TypeId, std::function<cpp_conv::resources::ResourceAsset*(FileData&)>*> g_typeHandlers;
static std::map<TypeId, LoadedAssetContainer*> g_loadedTypes = {};
static constexpr std::chrono::steady_clock::duration g_cacheInvalidationTime(std::chrono::seconds(10));

namespace
{
    std::mutex& getStateMutex()
    {
        static std::mutex s_stateMutex;
        return s_stateMutex;
    }

    std::function<cpp_conv::resources::ResourceAsset*(FileData&)>* getTypeHandler(const std::type_info& type)
    {
        // No need to lock here, this is only called in the context of an existing lock
        const auto iter = g_typeHandlers.find(type.hash_code());
        if (iter == g_typeHandlers.end())
        {
            return nullptr;
        }

        return iter->second;
    }
}

void cpp_conv::resources::resource_manager::registerTypeHandler(const std::type_info& type,
                                                                std::function<ResourceAsset*(FileData&)> fHandler)
{
    std::lock_guard<std::mutex> lock(getStateMutex());
    g_typeHandlers[type.hash_code()] = new std::function<ResourceAsset*(FileData&)>(fHandler);
    g_loadedTypes[type.hash_code()] = new LoadedAssetContainer();
}

cpp_conv::resources::resource_manager::FileData getFileData(cpp_conv::resources::registry::RegistryId kAssetId)
{
    PROFILE_FUNC();
    const auto rootPath = std::filesystem::current_path();
    const auto filePath = rootPath
        /
        "data" /
        cpp_conv::resources::registry::c_Files[kAssetId.m_Value].m_Path;

    std::ifstream file;
    file.open(filePath, std::ios::binary | std::ios::ate);
    if (file.fail())
    {
        return {RegistryId::Invalid(), nullptr, 0};
    }

    // ReSharper disable once CppRedundantCastExpression
    const uint32_t uiLength = static_cast<uint32_t>(file.tellg());

    // ReSharper disable once CppRedundantCastExpression
    file.seekg((std::streamoff)0, std::ios::beg);

    if (uiLength == 0)
    {
        return {RegistryId::Invalid(), nullptr, 0};
    }

    auto pData = new uint8_t[uiLength];
    file.read(reinterpret_cast<char*>(pData), uiLength);

    return {kAssetId, pData, uiLength};
}

cpp_conv::resources::AssetPtr<cpp_conv::resources::ResourceAsset> cpp_conv::resources::resource_manager::loadAsset(
    const std::type_info& type,
    registry::RegistryId kAssetId)
{
    PROFILE_FUNC();
    std::lock_guard<std::mutex> lock(getStateMutex());
    const auto loadedContainerIter = g_loadedTypes.find(type.hash_code());
    if (loadedContainerIter == g_loadedTypes.end())
    {
        return nullptr;
    }

    const auto existingAssetEntry = loadedContainerIter->second->find(kAssetId);
    if (existingAssetEntry != loadedContainerIter->second->end())
    {
        existingAssetEntry->second.m_lastAccess = std::chrono::steady_clock::now();
        return existingAssetEntry->second.m_ptr;
    }

    const std::function<ResourceAsset*(FileData&)>* fHandler = getTypeHandler(type);
    if (!fHandler || !(*fHandler))
    {
        return nullptr;
    }

    FileData kFileData = getFileData(kAssetId);
    if (!kFileData.m_pData)
    {
        return nullptr;
    }

    ResourceAsset* pAsset = (*fHandler)(kFileData);

    delete[] kFileData.m_pData;

    if (!pAsset)
    {
        return nullptr;
    }

    AssetPtr<ResourceAsset> pSharedAsset(pAsset);
    (*loadedContainerIter->second)[kAssetId] = {pSharedAsset, std::chrono::steady_clock::now()};
    return pSharedAsset;
}

cpp_conv::resources::AssetPtr<cpp_conv::resources::ResourceAsset>
cpp_conv::resources::resource_manager::loadAssetUncached(const std::type_info& type, registry::RegistryId kAssetId)
{
    const std::function<ResourceAsset*(FileData&)>* fHandler = getTypeHandler(type);
    if (!fHandler || !(*fHandler))
    {
        return nullptr;
    }

    FileData kFileData = getFileData(kAssetId);
    if (!kFileData.m_pData)
    {
        return nullptr;
    }

    ResourceAsset* pAsset = (*fHandler)(kFileData);
    delete[] kFileData.m_pData;

    AssetPtr<ResourceAsset> pSharedAsset(pAsset);
    return pSharedAsset;
}


void cpp_conv::resources::resource_manager::updatePersistenceStore()
{
    std::lock_guard<std::mutex> lock(getStateMutex());

    const auto now = std::chrono::steady_clock::now();
    for (const auto& type : g_loadedTypes | std::views::values)
    {
        if (!type)
        {
            continue;
        }

        auto containerIterator = type->begin();
        while (containerIterator != type->end())
        {
            if (containerIterator->second.m_ptr.use_count() == 1 && containerIterator->second.m_lastAccess - now >
                g_cacheInvalidationTime)
            {
                containerIterator = type->erase(containerIterator);
            }
            else
            {
                ++containerIterator;
            }
        }
    }
}
