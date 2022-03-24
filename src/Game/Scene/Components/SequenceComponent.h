#pragma once

#include "FixedCircularBuffer.h"
#include <cstdint>
#include <AtlasScene/ECS/Entity.h>
#include <AtlasScene/ECS/Entity.h>

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
            {}

            uint64_t m_Lanes = {};
            uint64_t m_RealizedMovements = {};
            uint64_t m_HasOverridePosition = {};
            FixedCircularBuffer<SlotItem> m_Items;
        };
        struct PendingState
        {
            explicit PendingState(const uint8_t uiConveyorLength) : m_NewItems{(uiConveyorLength)}
            {}

            uint64_t m_PendingInsertions = {};
            uint64_t m_PendingMoves = {};
            uint64_t m_PendingClears = {};
            uint64_t m_PendingRemovals = {};
            FixedCircularBuffer<SlotItem> m_NewItems;
        };

        SequenceComponent(
            uint8_t length,
            atlas::scene::EntityId headConveyor,
            Eigen::Vector2f laneOneVisualPosition,
            Eigen::Vector2f laneTwoVisualPosition,
            Eigen::Vector2f unitDirection,
            uint32_t moveTick)
            : m_Length(length)
            , m_HeadConveyor(headConveyor)
            , m_LaneOneVisualPosition(laneOneVisualPosition)
            , m_LaneTwoVisualPosition(laneTwoVisualPosition)
            , m_UnitDirection(unitDirection)
            , m_MoveTick(moveTick)             
        {
        }

        uint8_t m_Length;

        atlas::scene::EntityId m_HeadConveyor;

        Eigen::Vector2f m_LaneOneVisualPosition;

        Eigen::Vector2f m_LaneTwoVisualPosition;

        Eigen::Vector2f m_UnitDirection;

        uint32_t m_MoveTick;
        /*


        bool m_bNeedsRealization = false;
             */
   //std::array<RealizedState, c_conveyorChannels> m_RealizedStates;

        //std::array<PendingState, c_conveyorChannels> m_PendingStates;
    };
}
