#pragma once

#include <string>
#include "AtlasResource/AssetPtr.h"
#include "AssetRegistry.h"
#include "DataField.h"
#include "DataId.h"
#include "Serializable.h"
#include "TileAsset.h"
#include "TileAsset.h"
#include "TomlSerializer.h"

namespace atlas
{
    namespace render
    {
        class ModelAsset;
    }
}

namespace cpp_conv
{
    class FactoryDefinition : public Serializable<FactoryDefinition, TomlSerializer, atlas::resource::ResourceAsset>
    {
        inline static TomlSerializer::Config ms_ConveyorConfig = {"factory"};
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
        [[nodiscard]] Eigen::Vector3i GetSize() const { return m_Size.m_Value; }
        [[nodiscard]] uint32_t GetProductionRate() const { return m_ProductionRate.m_Value; }
        [[nodiscard]] bool HasOwnOutputPipe() const { return m_OutputPipe.m_bIsSet; }
        [[nodiscard]] Eigen::Vector3i GetOutputPipe() const { return m_OutputPipe.m_Value; }
        [[nodiscard]] RecipeId GetProducedRecipe() const { return m_ProducedRecipe.m_Value; }

        [[nodiscard]] atlas::resource::AssetPtr<atlas::render::ModelAsset> GetModel() const;

    private:
        DataField<FactoryId, "id"> m_InternalId{};
        DataField<std::string, "name"> m_Name{};
        DataField<atlas::resource::BundleRegistryId, "asset"> m_AssetId{atlas::resource::BundleRegistryId::Invalid()};
        DataField<RecipeId, "recipe"> m_ProducedRecipe{};
        DataField<uint32_t, "rate"> m_ProductionRate{};

        DataField<Eigen::Vector3i, "size"> m_Size{};
        DataField<Eigen::Vector3i, "output", false> m_OutputPipe{};
    };
}
