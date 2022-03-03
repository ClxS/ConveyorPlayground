#pragma once

#include "Entity.h"
#include "Conveyor.h"
#include "EntityGrid.h"
#include "DataId.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    constexpr int c_maxInserterLength = 6;

    class Inserter : public Entity
    { 
    public:
        Inserter(Vector3 position, Vector3 size, Direction direction, InserterId inserterId);

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kRenderContext) const override;

        [[nodiscard]] Direction GetDirection() const override { return m_direction; }

        [[nodiscard]] const char* GetName() const override { return "Inserter"; }
        [[nodiscard]] std::string GetDescription() const override;

    private:
        enum class State
        {
            WaitingPickup,
            Transfering,
            Cooldown
        };

        InserterId m_inserterId;
        Direction m_direction;

        uint32_t m_transitTime;
        uint32_t m_cooldownTime;
        bool m_supportsStacks;

        ItemId m_currentItem;
        uint32_t m_currentStackSize;

        uint64_t m_uiTicksRemainingInState;
        State m_eCurrentState;

        bool TryGrabItem(const SceneContext& kContext);
        bool TryInsertItem(const SceneContext& kContext);
    };
}
