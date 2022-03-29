#pragma once

#include <string>

#include "AssetPtr.h"
#include "AssetRegistry.h"
#include "DataId.h"
#include "ResourceAsset.h"
#include "Serializable.h"
#include "TileAsset.h"
#include "TileAsset.h"
#include "TomlSerializer.h"

namespace cpp_conv
{
    class ItemDefinition : public Serializable<ItemDefinition, TomlSerializer, resources::ResourceAsset>
    {
        inline static TomlSerializer::Config ms_ConveyorConfig = {"item"};
    public:
        ItemDefinition()
            : Serializable(
                ms_ConveyorConfig,
                {
                    &m_InternalId,
                    &m_Name,
                    &m_Description,
                    &m_AssetId,
                })
        {
        }

        ItemId GetInternalId() const { return m_InternalId.m_Value; }

        const std::string& GetName() const { return m_Name.m_Value; }
        const std::string& GetDescription() const { return m_Description.m_Value; }
        resources::AssetPtr<resources::TileAsset> GetTile() const;

    private:
        DataField<ItemId, "id"> m_InternalId{};
        DataField<std::string, "name"> m_Name{};
        DataField<std::string, "description", false> m_Description{};
        DataField<resources::registry::RegistryId, "asset"> m_AssetId{resources::registry::RegistryId::Invalid()};

        mutable resources::AssetPtr<resources::TileAsset> m_pTile;
    };
}
