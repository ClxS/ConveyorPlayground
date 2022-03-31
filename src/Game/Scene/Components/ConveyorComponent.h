#pragma once
#include <Eigen/Core>

#include "DataId.h"
#include "Direction.h"

#include "AtlasScene/ECS/Entity.h"

namespace cpp_conv::components
{
    constexpr int c_conveyorChannels = 2;
    constexpr int c_conveyorChannelSlots = 2;

    // This empty component essentially acts as a tag for components which do not form part of a sequence so need
    // to be individually processed
    struct IndividuallyProcessableConveyorComponent {};

    struct ConveyorComponent
    {
        struct PlacedItem
        {
            ItemId m_Item;
            std::optional<Eigen::Vector2f> m_PreviousPosition;
            bool m_bShouldAnimate;
        };

        struct Lane
        {
            PlacedItem m_Item;
            Eigen::Vector2f m_VisualPosition;
        };

        struct Channel
        {
            int m_ChannelLane;
            int m_LaneLength = 2;
            std::array<Lane, c_conveyorChannelSlots + 1> m_pSlots;
            std::array<PlacedItem, c_conveyorChannelSlots + 1> m_pPendingItems;
        };

        bool m_bIsCorner;
        bool m_bIsClockwise;
        int m_InnerMostChannel;
        Direction m_CornerDirection;
        std::array<Channel, c_conveyorChannels> m_Channels;

        uint32_t m_CurrentTick = 0;
        uint32_t m_MoveTick = 10;

        atlas::scene::EntityId m_Sequence;
        uint8_t m_SequenceIndex;
    };
}
