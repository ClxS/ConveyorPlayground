#include "ResourceManager.h"
#include "ResourceAsset.h"
#include "Profiler.h"

#include "MapLoadHandler.h"
#include "TileLoadHandler.h"
#include "ItemRegistry.h"

#include <map>
#include <memory>
#include <mutex>
#include <fstream>
#include <chrono>

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
static std::map<TypeId, std::function<cpp_conv::resources::ResourceAsset* (FileData&)>*> g_typeHandlers;
static std::map<TypeId, LoadedAssetContainer*> g_loadedTypes = {};
static constexpr std::chrono::steady_clock::duration g_cacheInvalidationTime(std::chrono::seconds(10));

namespace
{
    std::mutex& getStateMutex()
    {
        static std::mutex s_stateMutex;
        return s_stateMutex;
    }

    std::function<cpp_conv::resources::ResourceAsset* (FileData&)>* getTypeHandler(const std::type_info& type)
    {
        // No need to lock here, this is only called in the context of an existing lock
        auto iter = g_typeHandlers.find(type.hash_code());
        if (iter == g_typeHandlers.end())
        {
            return nullptr;
        }

        return iter->second;
    }
}

void cpp_conv::resources::resource_manager::initialize()
{
    cpp_conv::resources::registerMapLoadHandler();
    cpp_conv::resources::registerTileLoadHandler();
    cpp_conv::resources::registerItemHandler();
}

void cpp_conv::resources::resource_manager::registerTypeHandler(const std::type_info& type, std::function<cpp_conv::resources::ResourceAsset*(FileData&)> fHandler)
{
    std::lock_guard<std::mutex> lock(getStateMutex());
    g_typeHandlers[type.hash_code()] = new std::function<cpp_conv::resources::ResourceAsset* (FileData&)>(fHandler);
    g_loadedTypes[type.hash_code()] = new LoadedAssetContainer();
}

cpp_conv::resources::resource_manager::FileData getFileData(cpp_conv::resources::registry::RegistryId kAssetId)
{
    PROFILE_FUNC();
    constexpr const char* c_fileExt = ".txt";
    std::string path = "data/";
    path += cpp_conv::resources::registry::c_szCategoryPaths[kAssetId.m_category][kAssetId.m_index];
    path += c_fileExt;
     
    std::ifstream file;
    file.open(path, std::ios::binary | std::ios::ate);
    if (file.fail())
    {
        return {};
    }

    size_t uiLength = file.tellg();
    file.seekg(0, std::ios::beg);

    if (uiLength == 0)
    {
        return {};
    }

    uint8_t* pData = new uint8_t[uiLength];
    file.read(reinterpret_cast<char*>(pData), uiLength);

    return { pData, uiLength };
}

cpp_conv::resources::AssetPtr<cpp_conv::resources::ResourceAsset> cpp_conv::resources::resource_manager::loadAsset(const std::type_info& type, cpp_conv::resources::registry::RegistryId kAssetId)
{
    PROFILE_FUNC();
    std::lock_guard<std::mutex> lock(getStateMutex());
    auto loadedContainerIter = g_loadedTypes.find(type.hash_code());
    if (loadedContainerIter == g_loadedTypes.end())
    {
        return nullptr;
    }

    auto existingAssetEntry = loadedContainerIter->second->find(kAssetId);
    if (existingAssetEntry != loadedContainerIter->second->end())
    {
        existingAssetEntry->second.m_lastAccess = std::chrono::steady_clock::now();
        return existingAssetEntry->second.m_ptr;
    }

    std::function<ResourceAsset* (FileData&)>* fHandler = getTypeHandler(type);
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

    cpp_conv::resources::AssetPtr<cpp_conv::resources::ResourceAsset> pSharedAsset(pAsset);
    (*loadedContainerIter->second)[kAssetId] = { pSharedAsset, std::chrono::steady_clock::now() };
    return pSharedAsset;
}

void cpp_conv::resources::resource_manager::updatePersistenceStore()
{
    std::lock_guard<std::mutex> lock(getStateMutex());

    auto now = std::chrono::steady_clock::now();
    for (auto& pTypeContainer : g_loadedTypes)
    {
        if (!pTypeContainer.second)
        {
            continue;
        }

        auto containerIterator = pTypeContainer.second->begin();
        while (containerIterator != pTypeContainer.second->end())
        {
            if (containerIterator->second.m_ptr.use_count() == 1 && containerIterator->second.m_lastAccess - now > g_cacheInvalidationTime)
            {
                containerIterator = pTypeContainer.second->erase(containerIterator);
            }
            else
            {
                ++containerIterator;
            }
        }
    }
}
