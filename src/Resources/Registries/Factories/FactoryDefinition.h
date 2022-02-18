#pragma once

#include <string>
#include "ResourceAsset.h"
#include "TileAsset.h"
#include "DataId.h"
#include "AssetPtr.h"
#include "AssetRegistry.h"
#include "TileAsset.h"
#include "Vector3.h"
#include "DataField.h"
#include "Serializable.h"
#include "TomlSerializer.h"

namespace cpp_conv
{
    class FactoryDefinition : public Serializable<FactoryDefinition, TomlSerializer, resources::ResourceAsset>
    {
        inline static TomlSerializer::Config ms_ConveyorConfig = { "factory" };
    public:
        FactoryDefinition()
            : Serializable(
                ms_ConveyorConfig,
                {
                    &m_InternalId,
                    &m_Name,
                    &m_AssetId,
                    &m_ProducedRecipe,
                    &m_ProductionRate,
                    &m_Size,
                    &m_OutputPipe
                })
        {
        }

        [[nodiscard]] FactoryId GetInternalId() const { return m_InternalId.m_Value; }
        [[nodiscard]] const std::string& GetName() const { return m_Name.m_Value; }
        [[nodiscard]] Vector3 GetSize() const { return m_Size.m_Value; }
        [[nodiscard]] uint32_t GetProductionRate() const { return m_ProductionRate.m_Value; }
        [[nodiscard]] bool HasOwnOutputPipe() const { return m_OutputPipe.m_bIsSet; }
        [[nodiscard]] Vector3 GetOutputPipe() const { return m_OutputPipe.m_Value; }
        [[nodiscard]] RecipeId GetProducedRecipe() const { return m_ProducedRecipe.m_Value; }

        [[nodiscard]] resources::AssetPtr<resources::TileAsset> GetTile() const;

    private:
        DataField<FactoryId, "id"> m_InternalId{};
        DataField<std::string, "name"> m_Name{};
        DataField<resources::registry::RegistryId, "asset"> m_AssetId{resources::registry::RegistryId::Invalid()};
        DataField<RecipeId, "recipe"> m_ProducedRecipe{};
        DataField<uint32_t, "rate"> m_ProductionRate{};

        DataField<Vector3, "size"> m_Size{};
        DataField<Vector3, "output", false> m_OutputPipe{};
    };
}
