#pragma once

#include "Renderer.h"
#include "Entity.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    class Producer : public Entity
    { 
    public:
        Producer(int x, int y, Direction direction, ItemId pItem, uint64_t productionRate);

        bool IsReadyToProduce() const;

        ItemId ProduceItem();

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;

        Direction GetDirection() const override { return m_direction; }
    private:
        ItemId m_pItem;
        Direction m_direction;

        uint64_t m_uiTick;
        uint64_t m_productionRate;
        bool m_bProductionReady;
    };
}
