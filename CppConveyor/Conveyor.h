#pragma once

#include <array>

#include "Entity.h"
#include "Enums.h"
#include "Item.h"

namespace cpp_conv
{
    constexpr int c_conveyorChannels = 2;
    constexpr int c_conveyorChannelSlots = 2;

    class Conveyor : public Entity
    {
    public:
        struct Channel
        {
            struct Slot
            {
                enum ESlot
                {
                    FirstSlot = 0,
                    LastSlot = c_conveyorChannelSlots - 1
                };
            };

            Channel()
            {
                std::fill(std::begin(m_pItems), std::end(m_pItems), nullptr);
                std::fill(std::begin(m_pPendingItems), std::end(m_pPendingItems), nullptr);
            }

            std::array<Item*, c_conveyorChannelSlots> m_pItems;
            std::array<Item*, c_conveyorChannelSlots> m_pPendingItems;
        };

        Conveyor(int32_t x, int32_t y, Direction direction, Item* pItem = nullptr)
            : Entity(x, y, EntityKind::Conveyor)
            , m_direction(direction)
        {
        }

        Direction m_direction;

        std::array<Channel, c_conveyorChannels> m_pChannels;

        static_assert(c_conveyorChannels >= 1, "Conveyors must have at least once channel");
        static_assert(c_conveyorChannelSlots >= 1, "Conveyors channels must have at least once slot");
    };
}