#pragma once

#include <functional>
#include <mutex>

#include "FileData.h"
#include "RegistryBundle.h"
#include "AtlasCore/TemplatedUniquenessCounter.h"
#include "AtlasResource/AssetPtr.h"

namespace atlas::resource
{
    struct ResourceAsset;

    class ResourceLoader
    {
    public:
        using LoaderFunc = std::function<AssetPtr<ResourceAsset>(const FileData&)>;

        template<typename TBundle>
        static void RegisterBundle()
        {
            std::lock_guard lock(ms_StateMutex);
            const int32_t index = core::TemplatedUniquenessCounter<TBundle, ResourceLoader>::Ensure();
            if (index >= ms_Bundles.size())
            {
                ms_Bundles.resize(index + 1);
            }

            ms_Bundles[index] = { std::make_unique<TBundle>() };
        }

        template<typename TType>
        static void RegisterTypeHandler(LoaderFunc func)
        {
            std::lock_guard lock(ms_StateMutex);
            const int32_t typeIndex = core::TemplatedUniquenessCounter<TType, ResourceLoader>::Ensure();
            if (typeIndex >= ms_TypeHandlers.size())
            {
                ms_TypeHandlers.resize(typeIndex + 1);
            }

            ms_TypeHandlers[typeIndex] = std::move(func);
        }

        template<typename TBundle, typename TType>
        static AssetPtr<ResourceAsset> LoadAsset(const RegistryId registryId)
        {
            const int32_t bundleId = core::TemplatedUniquenessCounter<TBundle, ResourceLoader>::Ensure();
            const int32_t typeId = core::TemplatedUniquenessCounter<TType, ResourceLoader>::Ensure();

            const auto pAsset = LoadAsset(typeId, bundleId, registryId, false);
            if (!pAsset)
            {
                return nullptr;
            }

            return std::reinterpret_pointer_cast<TType>(pAsset);
        }

        template<typename TBundle, typename TType>
        static AssetPtr<ResourceAsset> LoadAssetUncached(const RegistryId registryId)
        {
            const int32_t bundleId = core::TemplatedUniquenessCounter<TBundle, ResourceLoader>::Ensure();
            const int32_t typeId = core::TemplatedUniquenessCounter<TType, ResourceLoader>::Ensure();

            const auto pAsset = LoadAsset(typeId, bundleId, registryId, true);
            if (!pAsset)
            {
                return nullptr;
            }

            return std::reinterpret_pointer_cast<TType>(pAsset);
        }

    private:
        struct LoadedAsset
        {
            AssetPtr<ResourceAsset> m_Asset;
            std::chrono::steady_clock::time_point m_LastAccess;
        };
        
        struct BundleCache
        {
            std::vector<LoadedAsset> m_Entries;
        };

        struct Bundle
        {
            std::unique_ptr<RegistryBundle> m_Bundle;
            BundleCache m_BundleCache;
        };

        inline static std::mutex ms_StateMutex;

        inline static std::vector<Bundle> ms_Bundles;
        inline static std::vector<LoaderFunc> ms_TypeHandlers;

        static AssetPtr<ResourceAsset> LoadAsset(int32_t typeId, int32_t bundleId, RegistryId registryId, bool uncached);
    };
}
