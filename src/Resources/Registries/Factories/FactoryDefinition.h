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
            bool bHasOwnOutputPipe,
            RecipeId producedRecipe)
            : m_internalId(internalId)
            , m_registryId(registryId)
            , m_strName(std::move(strName))
            , m_uiProductionRate(productionRate)
            , m_bHasOwnOutputPipe(bHasOwnOutputPipe)
            , m_producedRecipe(producedRecipe)
        {
        }

        cpp_conv::FactoryId GetInternalId() const { return m_internalId; }
        const std::string& GetName() const { return m_strName; }
        uint32_t GetProductionRate() const { return m_uiProductionRate; }
        bool HasOwnOutputPipe() const { return m_bHasOwnOutputPipe; }
        RecipeId GetProducedRecipe() const { return m_producedRecipe; }

        cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> GetTile() const;

    private:
        cpp_conv::FactoryId m_internalId;
        cpp_conv::resources::registry::RegistryId m_registryId;
        std::string m_strName;
        uint32_t m_uiProductionRate;
        bool m_bHasOwnOutputPipe;
        RecipeId m_producedRecipe;
    };
}