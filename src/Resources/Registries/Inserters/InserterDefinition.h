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
    class InserterDefinition : public cpp_conv::resources::ResourceAsset
    {
    public:
        InserterDefinition(
            cpp_conv::InserterId internalId,
            cpp_conv::resources::registry::RegistryId registryId,
            std::string strName,
            uint32_t uiTransitTime,
            uint32_t uiCooldownTime,
            bool bSupportsStacks)
            : m_internalId(internalId)
            , m_registryId(registryId)
            , m_strName(std::move(strName))
            , m_uiTransitTime(uiTransitTime)
            , m_uiCooldownTime(uiCooldownTime)
            , m_bSupportsStacks(bSupportsStacks)
        {
        }

        [[nodiscard]] cpp_conv::InserterId GetInternalId() const { return m_internalId; }

        [[nodiscard]] const std::string& GetName() const { return m_strName; }
        [[nodiscard]] cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> GetTile() const;

        [[nodiscard]] uint32_t GetTransitTime() const { return m_uiTransitTime; }
        [[nodiscard]] uint32_t GetCooldownTime() const { return m_uiCooldownTime; }
        [[nodiscard]] bool IsStackingInserter() const { return m_bSupportsStacks; }

    private:
        cpp_conv::InserterId m_internalId;
        cpp_conv::resources::registry::RegistryId m_registryId;
        std::string m_strName;

        uint32_t m_uiTransitTime;
        uint32_t m_uiCooldownTime;
        bool m_bSupportsStacks;
    };
}
