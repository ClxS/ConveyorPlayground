#pragma once

#include <array>

#include "Renderer.h"
#include "Entity.h"
#include "Enums.h"
#include "Item.h"

namespace cpp_conv
{
    constexpr int c_conveyorChannels = 2;
    constexpr int c_conveyorChannelSlots = 2;

    class Sequence;

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

            Channel();

            std::array<Item*, c_conveyorChannelSlots> m_pItems;
            std::array<Item*, c_conveyorChannelSlots> m_pPendingItems;
        };

        Conveyor(int32_t x, int32_t y, Direction direction, Item* pItem = nullptr);

        Direction m_direction;
        int m_pSequenceId;

        std::array<Channel, c_conveyorChannels> m_pChannels;

        void Tick(cpp_conv::grid::EntityGrid& grid);
        void Draw(HANDLE hConsole, cpp_conv::renderer::ScreenBuffer screenBuffer, cpp_conv::grid::EntityGrid& grid, int x, int y) const;

        static_assert(c_conveyorChannels >= 1, "Conveyors must have at least once channel");
        static_assert(c_conveyorChannelSlots >= 1, "Conveyors channels must have at least once slot");
    };
}