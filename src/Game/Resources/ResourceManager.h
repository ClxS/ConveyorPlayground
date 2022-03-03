#pragma once

#include <functional>
#include <memory>

#include "AssetPtr.h"
#include "AssetRegistry.h"
#include "ResourceAsset.h"

namespace cpp_conv::resources::resource_manager
{
    struct FileData
    {
        registry::RegistryId m_registryId;
        uint8_t* m_pData;
        uint64_t m_uiSize;
    };

    void registerTypeHandler(const std::type_info& type, std::function<ResourceAsset*(FileData&)> fHandler);
    void updatePersistenceStore();

    AssetPtr<ResourceAsset> loadAssetUncached(const std::type_info& type, registry::RegistryId kAssetId);
    AssetPtr<ResourceAsset> loadAsset(const std::type_info& type, registry::RegistryId kAssetId);

    template<typename TType>
    void registerTypeHandler(std::function<ResourceAsset*(FileData& rData)> fHandler)
    {
        registerTypeHandler(typeid(TType), fHandler);
    }

    template<typename TType>
    AssetPtr<TType> loadAsset(registry::RegistryId kAssetId)
    {
        const auto pAsset = loadAsset(typeid(TType), kAssetId);
        if (!pAsset)
        {
            return nullptr;
        }

        return std::reinterpret_pointer_cast<TType>(pAsset);
    }

    template<typename TType>
    AssetPtr<TType> loadAssetUncached(registry::RegistryId kAssetId)
    {
        const auto pAsset = loadAssetUncached(typeid(TType), kAssetId);
        if (!pAsset)
        {
            return nullptr;
        }

        return std::reinterpret_pointer_cast<TType>(pAsset);
    }
}