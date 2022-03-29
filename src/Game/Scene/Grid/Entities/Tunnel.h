#pragma once

#include "Conveyor.h"
#include "Entity.h"
#include "EntityGrid.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    constexpr int c_maxUndergroundLength = 6;

    class Tunnel final : public Entity
    {
    public:
        Tunnel(Vector3 position, Vector3 size, Direction direction);

        void Tick(const SceneContext& kContext) override;
        void Realize() override;
        void Draw(RenderContext& kRenderContext) const override;
        [[nodiscard]] bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, InsertInfo insertInfo) override;

        [[nodiscard]] Direction GetDirection() const override { return m_Direction; }

        [[nodiscard]] const char* GetName() const override { return "Tunnel"; }
        [[nodiscard]] std::string GetDescription() const override { return ""; }

        [[nodiscard]] bool RequiresPlacementLocalityChecks() const override { return true; }
        void OnLocalityUpdate(const WorldMap& map) override;

    private:
        static inline constexpr uint32_t c_uiMaxLength = 7;

        Tunnel* m_pOtherSide;
        Sequence* m_pSequence;
        Direction m_Direction;

        const uint32_t m_uiMoveTick = 10;
    };
}
