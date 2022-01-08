#pragma once

#include "Renderer.h"
#include "Entity.h"
#include "DataId.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    class Factory : public Entity
    { 
    public:
        Factory(
            int x,
            int y,
            Direction direction,
            FactoryId factoryId);
        Factory(
            int x,
            int y,
            FactoryId factoryId);

        bool IsReadyToProduce() const;

        ItemId ProduceItem();

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;

        Direction GetDirection() const override { return m_direction; }
    private:
        FactoryId m_pFactoryId;
        ItemId m_pItem;
        Direction m_direction;

        uint64_t m_uiTick;
        uint64_t m_productionRate;
        bool m_bProductionReady;
    };
}
