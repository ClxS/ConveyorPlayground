#pragma once

#include <string>

#include "AssetPtr.h"
#include "AssetRegistry.h"
#include "ResourceAsset.h"
#include "TileAsset.h"
#include "DataId.h"
#include "TileAsset.h"

namespace cpp_conv
{
    class ItemDefinition : public resources::ResourceAsset
    {
    public:
        ItemDefinition(
            ItemId internalId,
            resources::registry::RegistryId registryId,
            resources::registry::RegistryId assetId,
            std::string strName)
            : m_internalId(internalId)
            , m_registryId(registryId)
            , m_AssetId(assetId)
            , m_strName(std::move(strName))
        {
        }

        ItemId GetInternalId() const { return m_internalId; }

        const std::string& GetName() const { return m_strName; }
        resources::AssetPtr<resources::TileAsset> GetTile() const;

    private:
        ItemId m_internalId;
        resources::registry::RegistryId m_registryId;
        resources::registry::RegistryId m_AssetId;
        std::string m_strName;

        mutable resources::AssetPtr<resources::TileAsset> m_pTile;
    };
}
