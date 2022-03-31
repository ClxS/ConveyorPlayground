#pragma once

#include "Conveyor.h"
#include "DataId.h"
#include "Entity.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    constexpr int c_maxInserterLength = 6;

    class Inserter : public Entity
    {
    public:
        Inserter(Eigen::Vector3i position, Eigen::Vector3i size, Direction direction, InserterId inserterId)
            : Entity(position, size, EntityKind::Conveyor)
            , m_inserterId{inserterId}
            , m_direction(direction)
        {
        }

        [[nodiscard]] Direction GetDirection() const { return m_direction; }

        [[nodiscard]] const char* GetName() const { return "Inserter"; }
        [[nodiscard]] std::string GetDescription() const;

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
