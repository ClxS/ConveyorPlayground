#pragma once

#include "Entity.h"
#include "Conveyor.h"
#include "Grid.h"

namespace cpp_conv
{
    class Item;
    struct SceneContext;
    struct RenderContext;

    constexpr int c_maxUndergroundLength = 6;

    class Underground : public Entity
    {
    public:
        Underground(int x, int y, Direction direction);

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;
        bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const Item* pItem, int iSourceChannel) override;

        Direction GetDirection() const override { return m_direction; }

    private:
        cpp_conv::grid::EntityGrid m_kLocalGrid;
        Direction m_direction;
        std::array<Conveyor, c_maxUndergroundLength> m_arrInternalConveyors;
        uint64_t m_uiTick;
    };
}
