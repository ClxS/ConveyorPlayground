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
    class FactoryDefinition : public cpp_conv::resources::ResourceAsset
    {
    public:
        FactoryDefinition(
            cpp_conv::FactoryId internalId,
            cpp_conv::resources::registry::RegistryId registryId,
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

        cpp_conv::FactoryId GetInternalId() const { return m_internalId; }
        const std::string& GetName() const { return m_strName; }
        Vector3 GetSize() const { return m_size; }
        uint32_t GetProductionRate() const { return m_uiProductionRate; }
        bool HasOwnOutputPipe() const { return m_bHasOwnOutputPipe; }
        Vector3 GetOutputPipe() const { return m_outputPipe; }
        RecipeId GetProducedRecipe() const { return m_producedRecipe; }

        cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> GetTile() const;

    private:
        cpp_conv::FactoryId m_internalId;
        cpp_conv::resources::registry::RegistryId m_registryId;
        std::string m_strName;
        Vector3 m_size;
        uint32_t m_uiProductionRate;
        bool m_bHasOwnOutputPipe;
        Vector3 m_outputPipe;
        RecipeId m_producedRecipe;
    };
}
