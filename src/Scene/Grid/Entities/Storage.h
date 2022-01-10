#pragma once

#include "Entity.h"

#include <vector>
#include "GeneralItemContainer.h"

namespace cpp_conv
{
    class Storage : public Entity
    {
    public:
        Storage(int x, int y, uint32_t uiMaxCapacity, uint32_t uiMaxStackSize);

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;
        bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, ItemId pItem, int iSourceChannel) override;

        bool SupportsProvidingItem() const override { return true; }
        bool TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem) override;

    private:
        GeneralItemContainer m_itemContainer;
    };
}
