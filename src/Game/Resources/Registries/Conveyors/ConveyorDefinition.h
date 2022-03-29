#pragma once

#include <string>
#include "AssetPtr.h"
#include "AssetRegistry.h"
#include "DataField.h"
#include "DataId.h"
#include "ResourceAsset.h"
#include "Serializable.h"
#include "TileAsset.h"
#include "TileAsset.h"
#include "TomlSerializer.h"

namespace cpp_conv
{
    class ConveyorDefinition final : public Serializable<ConveyorDefinition, TomlSerializer, resources::ResourceAsset>
    {
        inline static TomlSerializer::Config ms_ConveyorConfig = {"conveyor"};
    public:
        ConveyorDefinition()
            : Serializable(
                ms_ConveyorConfig,
                {&m_InternalId, &m_Name, &m_TickDelay})
        {
        }

        [[nodiscard]] ConveyorId GetInternalId() const { return m_InternalId.m_Value; }

        [[nodiscard]] const std::string& GetName() const { return m_Name.m_Value; }

    private:
        DataField<ConveyorId, "id", true> m_InternalId{};

        DataField<std::string, "name", true> m_Name{};

        DataField<int32_t, "tickDelay", true> m_TickDelay{};
    };
}
