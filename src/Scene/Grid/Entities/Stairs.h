#pragma once

#include "Entity.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    class Stairs : public Entity
    {
    public:
        Stairs(Vector3 position, Vector3 size, Direction direction, bool bIsUp);

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;
        bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, InsertInfo insertInfo) override;

        const char* GetName() const override { return m_bIsUp ? "Stairs (Up)" : "Stairs (Down)"; }
        std::string GetDescription() const override { return ""; }

        Direction GetDirection() const override { return m_direction; }

    private:
        ItemId m_pItem;
        uint64_t m_uiTick;
        Direction m_direction;

        bool m_bIsUp;
    };
}
