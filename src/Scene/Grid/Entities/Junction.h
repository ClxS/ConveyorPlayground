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

        bool AddItem(Item* pItem);

    private:
        Item* m_pItem;
        uint64_t m_uiTick;
    };
}