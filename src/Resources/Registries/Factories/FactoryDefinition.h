#pragma once

#include <string>
#include "ResourceAsset.h"
#include "TileAsset.h"
#include "DataId.h"
#include "ResourceRegistry.h"
#include "AssetPtr.h"
#include "TileAsset.h"
#include "Vector3.h"

namespace cpp_conv
{
    class FactoryDefinition : public resources::ResourceAsset
    {
    public:
        FactoryDefinition(
            FactoryId internalId,
            resources::registry::RegistryId registryId,
            std::string strName,
            Vector3 size,
            uint32_t productionRate,
            bool bHasOwnOutputPipe,
            Vector3 outputPipe,
            RecipeId producedRecipe)
            : m_internalId(internalId)
            , m_registryId(registryId)
            , m_strName(std::move(strName))
            , m_size(size)
            , m_uiProductionRate(productionRate)
            , m_bHasOwnOutputPipe(bHasOwnOutputPipe)
            , m_outputPipe(outputPipe)
            , m_producedRecipe(producedRecipe)
        {
        }

        [[nodiscard]] FactoryId GetInternalId() const { return m_internalId; }
        [[nodiscard]] const std::string& GetName() const { return m_strName; }
        [[nodiscard]] Vector3 GetSize() const { return m_size; }
        [[nodiscard]] uint32_t GetProductionRate() const { return m_uiProductionRate; }
        [[nodiscard]] bool HasOwnOutputPipe() const { return m_bHasOwnOutputPipe; }
        [[nodiscard]] Vector3 GetOutputPipe() const { return m_outputPipe; }
        [[nodiscard]] RecipeId GetProducedRecipe() const { return m_producedRecipe; }

        [[nodiscard]] resources::AssetPtr<resources::TileAsset> GetTile() const;

    private:
        FactoryId m_internalId;
        resources::registry::RegistryId m_registryId;
        std::string m_strName;
        Vector3 m_size;
        uint32_t m_uiProductionRate;
        bool m_bHasOwnOutputPipe;
        Vector3 m_outputPipe;
        RecipeId m_producedRecipe;
    };
}
