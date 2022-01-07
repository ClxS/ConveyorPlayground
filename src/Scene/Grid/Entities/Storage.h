#pragma once

#include "Entity.h"

#include <vector>

namespace cpp_conv
{
    class Storage : public Entity
    {
    public:
        Storage(int x, int y, uint32_t uiMaxCapacity, uint32_t uiMaxStackSize);

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;
        bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const Item* pItem, int iSourceChannel) override;

    private:
        struct ItemEntry
        {
            const Item* m_pItem;
            uint32_t m_pCount;
        };

        std::vector<ItemEntry> m_vItemEntries;
        uint32_t m_uiMaxCapacity;
        uint32_t m_uiMaxStackSize;
    };
}
