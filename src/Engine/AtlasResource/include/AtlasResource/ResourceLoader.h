#pragma once

#include <functional>
#include <mutex>

#include "FileData.h"
#include "RegistryBundle.h"
#include "AtlasCore/StringHash.h"
#include "AtlasCore/TemplatedUniquenessCounter.h"
#include "AtlasResource/AssetPtr.h"

namespace atlas::resource
{
    struct ResourceAsset;

    class ResourceLoader
    {
        struct BundleContext {};
        struct TypeContext {};
    public:
        using LoaderFunc = std::function<AssetPtr<ResourceAsset>(FileData&)>;

        template<typename TBundle>
        static void RegisterBundle()
        {
            std::lock_guard lock(ms_StateMutex);
            const int32_t index = core::TemplatedUniquenessCounter<TBundle, BundleContext>::Ensure();
            if (index >= ms_Bundles.size())
            {
                ms_Bundles.resize(index + 1);
            }

            const std::string id {TBundle::GetStringId()};
            ms_Bundles[index] = { std::make_unique<TBundle>(), {}, index, core::StringHash(id) };
            ms_Bundles[index].m_BundleCache.m_Entries.resize(TBundle::GetSize());
        }

        template<typename TType>
        static void RegisterTypeHandler(LoaderFunc func)
        {
            std::lock_guard lock(ms_StateMutex);
            const int32_t typeIndex = core::TemplatedUniquenessCounter<TType, TypeContext>::Ensure();
            if (typeIndex >= ms_TypeHandlers.size())
            {
                ms_TypeHandlers.resize(typeIndex + 1);
            }

            ms_TypeHandlers[typeIndex] = std::move(func);
        }

        template<typename TBundle, typename TType>
        static AssetPtr<TType> LoadAsset(const RegistryId registryId)
        {
            const int32_t bundleId = core::TemplatedUniquenessCounter<TBundle, BundleContext>::Ensure();
            const int32_t typeId = core::TemplatedUniquenessCounter<TType, TypeContext>::Ensure();

            const auto pAsset = LoadAsset(typeId, bundleId, registryId, false);
            if (!pAsset)
            {
                return nullptr;
            }

            return std::reinterpret_pointer_cast<TType>(pAsset);
        }

        template<typename TBundle, typename TType>
        static AssetPtr<TType> LoadAssetUncached(const RegistryId registryId)
        {
            const int32_t bundleId = core::TemplatedUniquenessCounter<TBundle, BundleContext>::Ensure();
            const int32_t typeId = core::TemplatedUniquenessCounter<TType, TypeContext>::Ensure();

            const auto pAsset = LoadAsset(typeId, bundleId, registryId, true);
            if (!pAsset)
            {
                return nullptr;
            }

            return std::reinterpret_pointer_cast<TType>(pAsset);
        }

        template<typename TType>
        static AssetPtr<TType> LoadAsset(const BundleRegistryId registryId)
        {
            const int32_t typeId = core::TemplatedUniquenessCounter<TType, TypeContext>::Ensure();
            const auto pAsset = LoadAsset(typeId, registryId.m_BundleIndex, registryId.m_BundleId, false);
            if (!pAsset)
            {
                return nullptr;
            }

            return std::reinterpret_pointer_cast<TType>(pAsset);
        }

        template<typename TType>
        static AssetPtr<TType> LoadAssetUncached(const BundleRegistryId registryId)
        {
            const int32_t typeId = core::TemplatedUniquenessCounter<TType, TypeContext>::Ensure();
            const auto pAsset = LoadAsset(typeId, registryId.m_BundleIndex, registryId.m_BundleId, true);
            if (!pAsset)
            {
                return nullptr;
            }

            return std::reinterpret_pointer_cast<TType>(pAsset);
        }

        static std::optional<BundleRegistryId> LookupId(const std::string_view id)
        {
            for (int32_t bundleIndex = 0; bundleIndex < static_cast<int32_t>(ms_Bundles.size()); bundleIndex++)
            {
                const auto& bundle = ms_Bundles[bundleIndex];
                if (!bundle.m_Bundle)
                {
                    continue;
                }

                auto result = bundle.m_Bundle->LookupId(id);
                if (result.has_value())
                {
                    return { BundleRegistryId{ result.value(), bundleIndex }};
                }
            }

            return {};
        }

        template<typename TBundle>
        static BundleRegistryId CreateBundleRegistryId(const RegistryId registryId)
        {
            const int32_t bundleId = core::TemplatedUniquenessCounter<TBundle, BundleContext>::Ensure();
            return { registryId, bundleId };
        }

        static BundleRegistryId CreateBundleRegistryId(const std::string_view& bundleStringId, const RegistryId registryId)
        {
            const int32_t bundleId = GetBundleIdFromStringIdentifier(bundleStringId);
            return { registryId, bundleId };
        }

        static int32_t GetBundleIdFromStringIdentifier(const std::string_view& bundleId)
        {
            const core::StringHashView hashView{bundleId};
            for(auto& bundle : ms_Bundles)
            {
                if (bundle.m_StringId == hashView)
                {
                    return bundle.m_Index;
                }
            }

            return -1;
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
            int32_t m_Index;
            core::StringHash m_StringId;
        };

        inline static std::recursive_mutex ms_StateMutex;

        inline static std::vector<Bundle> ms_Bundles;
        inline static std::vector<LoaderFunc> ms_TypeHandlers;

        static AssetPtr<ResourceAsset> LoadAsset(int32_t typeId, int32_t bundleId, RegistryId registryId, bool uncached);
    };
}
