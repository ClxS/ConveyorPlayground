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
    class RecipeDefinition : public cpp_conv::resources::ResourceAsset
    {
    public:
        struct RecipeItem
        {
            ItemId m_idItem;
            uint32_t m_uiCount;
        };

        RecipeDefinition(
            cpp_conv::RecipeId internalId,
            cpp_conv::resources::registry::RegistryId registryId,
            std::string name,
            uint32_t uiEffort,
            std::vector<RecipeItem> inputItems,
            std::vector<RecipeItem> outputItems)
            : m_internalId(internalId)
            , m_registryId(registryId)
            , m_strName(name)
            , m_uiEffort(uiEffort)
            , m_vInputItems(std::move(inputItems))
            , m_vOutputItems(std::move(outputItems))
        {
        }

        cpp_conv::RecipeId GetInternalId() const { return m_internalId; }
        const std::string GetName() const { return m_strName; }

        cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> GetTile() const;

        uint32_t GetEffort() const { return m_uiEffort; }
        const std::vector<RecipeItem>& GetInputItems() const { return m_vInputItems; }
        const std::vector<RecipeItem>& GetOutputItems() const { return m_vOutputItems; }

    private:
        cpp_conv::RecipeId m_internalId;
        cpp_conv::resources::registry::RegistryId m_registryId;
        std::string m_strName;

        uint32_t m_uiEffort;
        std::vector<RecipeItem> m_vInputItems;
        std::vector<RecipeItem> m_vOutputItems;
    };
}
