#pragma once

#include "AssetPtr.h"
#include "DataId.h"
#include "ResourceAsset.h"
#include "TileAsset.h"
#include "TileAsset.h"

#include <string>
#include <vector>

#include "AssetRegistry.h"
#include "Serializable.h"
#include "TomlSerializer.h"

namespace cpp_conv
{
    class RecipeDefinition : public Serializable<RecipeDefinition, TomlSerializer, resources::ResourceAsset>
    {
        inline static TomlSerializer::Config ms_ConveyorConfig = {"recipe"};
    public:
        struct RecipeItem
        {
            ItemId m_idItem;
            uint32_t m_uiCount;
        };

        RecipeDefinition()
            : Serializable(
                ms_ConveyorConfig,
                {
                    &m_InternalId,
                    &m_Name,
                    &m_Description,
                    &m_Effort,
                    &m_InputItems,
                    &m_OutputItems,
                })
        {
        }

        [[nodiscard]] RecipeId GetInternalId() const { return m_InternalId.m_Value; }
        [[nodiscard]] std::string GetName() const { return m_Name.m_Value; }
        [[nodiscard]] std::string GetDescription() const { return m_Description.m_Value; }
        [[nodiscard]] uint32_t GetEffort() const { return m_Effort.m_Value; }

        [[nodiscard]] const std::vector<RecipeItem>& GetInputItems() const { return m_InputItems.m_Value; }
        [[nodiscard]] const std::vector<RecipeItem>& GetOutputItems() const { return m_OutputItems.m_Value; }

    private:
        DataField<RecipeId, "id"> m_InternalId{};
        DataField<std::string, "name"> m_Name{};
        DataField<std::string, "description", false> m_Description{};
        DataField<uint32_t, "effort"> m_Effort{};

        DataField<std::vector<RecipeItem>, "input", false> m_InputItems;
        DataField<std::vector<RecipeItem>, "output"> m_OutputItems;
    };

    template <>
    struct TypedDataReader<RecipeDefinition::RecipeItem>
    {
        static bool Read(const toml::Table* value, const char* szPropertyName,
                         RecipeDefinition::RecipeItem& pTargetVariable);
    };

    template <>
    struct TypedDataReader<std::vector<RecipeDefinition::RecipeItem>>
    {
        static bool Read(const toml::Table* value, const char* szPropertyName,
                         std::vector<RecipeDefinition::RecipeItem>& pTargetVariable);
    };
}
