#pragma once

#include <cstdint>
#include <AtlasScene/ECS/Entity.h>
#include "FixedCircularBuffer.h"

namespace cpp_conv::components
{
    struct SequenceComponent
    {
        struct SlotItem
        {
            ItemId m_Item;
            std::optional<Eigen::Vector2f> m_Position{};
        };

        struct RealizedState
        {
            explicit RealizedState(const uint8_t uiConveyorLength) : m_Items{(uiConveyorLength)}
            {
            }

            uint64_t m_Lanes = {};
            uint64_t m_RealizedMovements = {};
            uint64_t m_HasOverridePosition = {};
            FixedCircularBuffer<SlotItem> m_Items;
        };

        struct PendingState
        {
            explicit PendingState(const uint8_t uiConveyorLength) : m_NewItems{(uiConveyorLength)}
            {
            }

            uint64_t m_PendingInsertions = {};
            uint64_t m_PendingMoves = {};
            uint64_t m_PendingClears = {};
            uint64_t m_PendingRemovals = {};
            FixedCircularBuffer<SlotItem> m_NewItems;
        };

        SequenceComponent(
            const uint8_t length,
            const atlas::scene::EntityId headConveyor,
            const Eigen::Vector2f laneOneVisualPosition,
            const Eigen::Vector2f laneTwoVisualPosition,
            const Eigen::Vector3f unitDirection,
            const uint32_t moveTick)
            : m_UnitDirection(unitDirection)
              , m_MoveTick(moveTick)
              , m_CurrentTick{0}
              , m_HeadConveyor(headConveyor)
              , m_Length(length)
              , m_LaneVisualOffsets{laneOneVisualPosition, laneTwoVisualPosition}
              , m_RealizedStates{RealizedState(length * 2), RealizedState(length * 2)}
              , m_PendingStates{PendingState(length * 2), PendingState(length * 2)}
        {
        }


        Eigen::Vector3f m_UnitDirection;

        uint32_t m_MoveTick;
        uint32_t m_CurrentTick;

        atlas::scene::EntityId m_HeadConveyor;

        uint8_t m_Length;

        std::array<Eigen::Vector2f, c_conveyorChannels> m_LaneVisualOffsets;
        std::array<RealizedState, c_conveyorChannels> m_RealizedStates;
        std::array<PendingState, c_conveyorChannels> m_PendingStates;
    };
}
