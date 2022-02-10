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
        [[nodiscard]] bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, InsertInfo insertInfo) override;

        [[nodiscard]] const char* GetName() const override { return m_bIsUp ? "Stairs (Up)" : "Stairs (Down)"; }
        [[nodiscard]] std::string GetDescription() const override { return ""; }

        [[nodiscard]] Direction GetDirection() const override { return m_direction; }

    private:
        ItemId m_pItem;
        uint64_t m_uiTick;
        Direction m_direction;

        bool m_bIsUp;
    };
}
