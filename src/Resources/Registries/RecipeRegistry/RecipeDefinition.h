#pragma once

#include "ResourceAsset.h"
#include "TileAsset.h"
#include "DataId.h"
#include "ResourceRegistry.h"
#include "AssetPtr.h"
#include "TileAsset.h"

#include <vector>
#include <string>

namespace cpp_conv
{
    class RecipeDefinition : public resources::ResourceAsset
    {
    public:
        struct RecipeItem
        {
            ItemId m_idItem;
            uint32_t m_uiCount;
        };

        RecipeDefinition(
            RecipeId internalId,
            resources::registry::RegistryId registryId,
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

        [[nodiscard]] RecipeId GetInternalId() const { return m_internalId; }
        [[nodiscard]] const std::string GetName() const { return m_strName; }

        [[nodiscard]] resources::AssetPtr<resources::TileAsset> GetTile() const;

        [[nodiscard]] uint32_t GetEffort() const { return m_uiEffort; }
        [[nodiscard]] const std::vector<RecipeItem>& GetInputItems() const { return m_vInputItems; }
        [[nodiscard]] const std::vector<RecipeItem>& GetOutputItems() const { return m_vOutputItems; }

    private:
        RecipeId m_internalId;
        resources::registry::RegistryId m_registryId;
        std::string m_strName;

        uint32_t m_uiEffort;
        std::vector<RecipeItem> m_vInputItems;
        std::vector<RecipeItem> m_vOutputItems;
    };
}
