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
    class FactoryDefinition : public cpp_conv::resources::ResourceAsset
    {
    public:
        FactoryDefinition(cpp_conv::FactoryId internalId, cpp_conv::resources::registry::RegistryId registryId, std::string strName, ItemId producedItem, uint32_t productionRate)
            : m_internalId(internalId)
            , m_registryId(registryId)
            , m_strName(std::move(strName))
            , m_producedItem(producedItem)
            , m_uiProductionRate(productionRate)
        {
        }

        cpp_conv::FactoryId GetInternalId() const { return m_internalId; }
        const std::string& GetName() const { return m_strName; }
        ItemId GetProducedItem() const { return m_producedItem; }
        uint32_t GetProductionRate() const { return m_uiProductionRate; }

        cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> GetTile() const;

    private:
        cpp_conv::FactoryId m_internalId;
        cpp_conv::resources::registry::RegistryId m_registryId;
        std::string m_strName;
        ItemId m_producedItem;
        uint32_t m_uiProductionRate;
    };
}
