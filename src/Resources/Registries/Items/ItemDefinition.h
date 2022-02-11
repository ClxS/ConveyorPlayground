#pragma once

#include <string>
#include "ResourceAsset.h"
#include "TileAsset.h"
#include "DataId.h"
#include "ResourceRegistry.h"
#include "AssetPtr.h"
#include "TileAsset.h"

namespace cpp_conv
{
    class ItemDefinition : public resources::ResourceAsset
    {
    public:
        ItemDefinition(ItemId internalId, resources::registry::RegistryId registryId, std::string strName)
            : m_internalId(internalId)
            , m_registryId(registryId)
            , m_strName(std::move(strName))
        {
        }

        ItemId GetInternalId() const { return m_internalId; }

        const std::string& GetName() const { return m_strName; }
        resources::AssetPtr<resources::TileAsset> GetTile() const;

    private:
        ItemId m_internalId;
        resources::registry::RegistryId m_registryId;
        std::string m_strName;

        mutable resources::AssetPtr<resources::TileAsset> m_pTile;
    };
}
