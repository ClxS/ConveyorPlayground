#pragma once
#include "Direction.h"
#include "ItemInstance.h"

namespace cpp_conv::components
{
    constexpr int c_conveyorChannels = 2;
    constexpr int c_conveyorChannelSlots = 2;

    struct ConveyorComponent
    {
        struct Lane
        {
            ItemInstance m_Item;
            Vector2F m_VisualPosition;
        };

        struct Channel
        {
            // ReSharper disable once CppNonExplicitConvertingConstructor
            Channel(int channelLane);

            const int m_ChannelLane;

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
    };
}
