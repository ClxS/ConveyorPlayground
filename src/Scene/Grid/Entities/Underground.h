#pragma once

#include "Entity.h"
#include "Conveyor.h"
#include "EntityGrid.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    constexpr int c_maxUndergroundLength = 6;

    class Underground : public Entity
    {
    public:
        Underground(Vector3 position, Vector3 size, Direction direction);

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;
        [[nodiscard]] bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, InsertInfo insertInfo) override;

        [[nodiscard]] Direction GetDirection() const override { return m_direction; }

        [[nodiscard]] const char* GetName() const override { return "Tunnel"; }
        [[nodiscard]] std::string GetDescription() const override { return ""; }

    private:
        Direction m_direction;
        std::array<Conveyor, c_maxUndergroundLength> m_arrInternalConveyors;
        uint64_t m_uiTick;
    };
}
