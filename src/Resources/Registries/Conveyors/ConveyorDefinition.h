#pragma once

#include <string>
#include "ResourceAsset.h"
#include "TileAsset.h"
#include "DataId.h"
#include "AssetPtr.h"
#include "AssetRegistry.h"
#include "TileAsset.h"

namespace cpp_conv
{
    class ConveyorDefinition : public resources::ResourceAsset
    {
    public:
        ConveyorDefinition(const ConveyorId internalId, const resources::registry::RegistryId registryId, std::string strName)
            : m_internalId(internalId)
            , m_registryId(registryId)
            , m_strName(std::move(strName))
        {
        }

        [[nodiscard]] ConveyorId GetInternalId() const { return m_internalId; }

        [[nodiscard]] const std::string& GetName() const { return m_strName; }

    private:
        ConveyorId m_internalId;
        resources::registry::RegistryId m_registryId;
        std::string m_strName;
    };
}
