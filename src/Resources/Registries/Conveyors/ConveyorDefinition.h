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
    class ConveyorDefinition : public cpp_conv::resources::ResourceAsset
    {
    public:
        ConveyorDefinition(cpp_conv::ConveyorId internalId, cpp_conv::resources::registry::RegistryId registryId, std::string strName, wchar_t displayIcon)
            : m_internalId(internalId)
            , m_registryId(registryId)
            , m_strName(std::move(strName))
            , m_displayIcon(displayIcon)
        {
        }

        cpp_conv::ConveyorId GetInternalId() const { return m_internalId; }

        const std::string& GetName() const { return m_strName; }
        cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> GetTile() const;

    private:
        cpp_conv::ConveyorId m_internalId;
        cpp_conv::resources::registry::RegistryId m_registryId;
        std::string m_strName;
        wchar_t m_displayIcon;
    };
}
