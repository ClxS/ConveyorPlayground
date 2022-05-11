#pragma once

#include <string>
#include "AtlasResource/AssetPtr.h"
#include "AssetRegistry.h"
#include "DataId.h"
#include "Serializable.h"
#include "TileAsset.h"
#include "TomlSerializer.h"

namespace cpp_conv
{
    class InserterDefinition : public Serializable<InserterDefinition, TomlSerializer, atlas::resource::ResourceAsset>
    {
        inline static TomlSerializer::Config ms_ConveyorConfig = {"inserter"};
    public:
        InserterDefinition()
            : Serializable(
                ms_ConveyorConfig,
                {
                    &m_InternalId,
                    &m_Name,
                    &m_AssetId,
                    &m_TransitTime,
                    &m_CooldownTime,
                    &m_bSupportsStacks,
                })
        {
        }

        [[nodiscard]] InserterId GetInternalId() const { return m_InternalId.m_Value; }

        [[nodiscard]] const std::string& GetName() const { return m_Name.m_Value; }
        [[nodiscard]] atlas::resource::AssetPtr<resources::TileAsset> GetTile() const;

        [[nodiscard]] uint32_t GetTransitTime() const { return m_TransitTime.m_Value; }
        [[nodiscard]] uint32_t GetCooldownTime() const { return m_CooldownTime.m_Value; }
        [[nodiscard]] bool IsStackingInserter() const { return m_bSupportsStacks.m_Value; }

    private:
        DataField<InserterId, "id"> m_InternalId{};
        DataField<std::string, "name"> m_Name{};
        DataField<atlas::resource::BundleRegistryId, "asset"> m_AssetId{atlas::resource::BundleRegistryId::Invalid()};
        DataField<uint32_t, "transitTime"> m_TransitTime{};
        DataField<uint32_t, "cooldownTime"> m_CooldownTime{};
        DataField<bool, "supportsStacks"> m_bSupportsStacks{};
    };
}
