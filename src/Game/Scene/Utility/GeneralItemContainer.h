#pragma once
#include "DataId.h"

#include <cstdint>
#include <vector>

namespace cpp_conv
{
    class GeneralItemContainer
    {
    public:
        GeneralItemContainer()
            : m_uiMaxCapacity{0}
            , m_uiMaxStackSize{0}
            , m_bUniqueStacksOnly{false}
        {
        }
        GeneralItemContainer(uint32_t uiMaxCapacity, uint32_t uiMaxStackSize, bool bUniqueStacksOnly);
        void Initialise(uint32_t uiMaxCapacity, uint32_t uiMaxStackSize, bool bUniqueStacksOnly);

        struct ItemEntry
        {
            ItemId m_pItem;
            uint32_t m_pCount;
        };

        bool TryTake(bool bSingle, std::tuple<ItemId, uint32_t>& outItem);
        bool TryTake(ItemId item, uint32_t count = 1);
        bool TryInsert(ItemId pItem, uint32_t count = 1);
        bool CouldInsert(ItemId pItem, uint32_t count = 1);
        bool HasItems(ItemId item, uint32_t count = 1);
        bool IsEmpty() const;

        std::vector<ItemEntry>& GetItems() { return m_vItemEntries; }
        [[nodiscard]] const std::vector<ItemEntry>& GetItems() const { return m_vItemEntries; }

        [[nodiscard]] std::string GetDescription() const;

        [[nodiscard]] uint32_t GetMaxCapacity() const { return m_uiMaxCapacity; }
        [[nodiscard]] uint32_t GetMaxStackSize() const { return m_uiMaxStackSize; }
        [[nodiscard]] bool OnlyAllowsUniqueStacks() const { return m_bUniqueStacksOnly; }

    private:
        uint32_t m_uiMaxCapacity;
        uint32_t m_uiMaxStackSize;
        bool m_bUniqueStacksOnly;

        std::vector<ItemEntry> m_vItemEntries;
    };
}
