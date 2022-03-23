#pragma once

#include "FixedCircularBuffer.h"
#include <cstdint>

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
            uint8_t length)
            : m_Length(length)
        {
        }

        uint8_t m_Length;

        /*atlas::scene::EntityId m_HeadConveyor;

        Eigen::Vector2f m_LaneOneVisualPosition;

        Eigen::Vector2f m_LaneTwoVisualPosition;

        Eigen::Vector2f m_UnitDirection;*/

        //uint32_t m_MoveTick;
        /*


        bool m_bNeedsRealization = false;
             */
   //std::array<RealizedState, c_conveyorChannels> m_RealizedStates;

        //std::array<PendingState, c_conveyorChannels> m_PendingStates;
    };
}
