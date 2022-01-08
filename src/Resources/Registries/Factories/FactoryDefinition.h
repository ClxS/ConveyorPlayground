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
        FactoryDefinition(
            cpp_conv::FactoryId internalId,
            cpp_conv::resources::registry::RegistryId registryId,
            std::string strName,
            uint32_t productionRate,
            ItemId producedItem)
            : m_internalId(internalId)
            , m_registryId(registryId)
            , m_strName(std::move(strName))
            , m_uiProductionRate(productionRate)
            , m_producedItem(producedItem)
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
        uint32_t m_uiProductionRate;
        ItemId m_producedItem;
    };
}
