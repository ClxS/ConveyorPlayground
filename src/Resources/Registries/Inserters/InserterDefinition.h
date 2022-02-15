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
    class InserterDefinition : public resources::ResourceAsset
    {
    public:
        InserterDefinition(
            InserterId internalId,
            resources::registry::RegistryId registryId,
            resources::registry::RegistryId assetId,
            std::string strName,
            uint32_t uiTransitTime,
            uint32_t uiCooldownTime,
            bool bSupportsStacks)
            : m_internalId(internalId)
            , m_registryId(registryId)
            , m_AssetId(assetId)
            , m_strName(std::move(strName))
            , m_uiTransitTime(uiTransitTime)
            , m_uiCooldownTime(uiCooldownTime)
            , m_bSupportsStacks(bSupportsStacks)
        {
        }

        [[nodiscard]] InserterId GetInternalId() const { return m_internalId; }

        [[nodiscard]] const std::string& GetName() const { return m_strName; }
        [[nodiscard]] resources::AssetPtr<resources::TileAsset> GetTile() const;

        [[nodiscard]] uint32_t GetTransitTime() const { return m_uiTransitTime; }
        [[nodiscard]] uint32_t GetCooldownTime() const { return m_uiCooldownTime; }
        [[nodiscard]] bool IsStackingInserter() const { return m_bSupportsStacks; }

    private:
        InserterId m_internalId;
        resources::registry::RegistryId m_registryId;
        resources::registry::RegistryId m_AssetId;
        std::string m_strName;

        uint32_t m_uiTransitTime;
        uint32_t m_uiCooldownTime;
        bool m_bSupportsStacks;
    };
}
