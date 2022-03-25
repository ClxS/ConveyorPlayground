#pragma once
#include <Eigen/Core>
#include "Direction.h"
#include "ItemInstance.h"

#include "AtlasScene/ECS/Entity.h"

namespace cpp_conv::components
{
    constexpr int c_conveyorChannels = 2;
    constexpr int c_conveyorChannelSlots = 2;

    struct ConveyorComponent
    {
        struct Lane
        {
            ItemInstance m_Item;
            Eigen::Vector2f m_VisualPosition;
        };

        struct Channel
        {
            int m_ChannelLane;
            int m_LaneLength = 2;
            std::array<Lane, c_conveyorChannelSlots + 1> m_pSlots;
            std::array<ItemInstance, c_conveyorChannelSlots + 1> m_pPendingItems;
        };

        bool m_bIsCorner;
        bool m_bIsClockwise;
        bool m_bIsCapped;
        int m_InnerMostChannel;
        Direction m_CornerDirection;
        std::array<Channel, c_conveyorChannels> m_Channels;

        int m_MoveTick = 10;

        atlas::scene::EntityId m_Sequence;
    };
}
