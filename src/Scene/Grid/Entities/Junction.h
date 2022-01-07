#pragma once

#include "Entity.h"

namespace cpp_conv
{
    class Item;
    struct SceneContext;
    struct RenderContext;

    class Junction : public Entity
    {
    public:
        Junction(int x, int y);

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;
        bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const Item* pItem, int iSourceChannel) override;


    private:
        const Item* m_pItem;
        uint64_t m_uiTick;
    };
}
