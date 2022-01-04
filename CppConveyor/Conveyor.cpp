#include "Conveyor.h"

void DrawConveyorBorder(HANDLE hConsole, cpp_conv::renderer::ScreenBuffer screenBuffer, int x, int y,
    int colour)
{
    for (int i = 0; i < 4; i++)
    {
        if (i != 0 && i != 3)
        {
            cpp_conv::renderer::setPixel(hConsole, screenBuffer, L'│', x, y + i, colour);
            cpp_conv::renderer::setPixel(hConsole, screenBuffer, L'│', x + 3, y + i, colour);
        }

        if (i != 0 && i != 3)
        {
            cpp_conv::renderer::setPixel(hConsole, screenBuffer, L'─', x + i, y, colour);
            cpp_conv::renderer::setPixel(hConsole, screenBuffer, L'─', x + i, y + 3, colour);
        }

        if (i == 0 || i == 3)
        {
            cpp_conv::renderer::setPixel(hConsole, screenBuffer, L'┼', x + i, y, colour);
            cpp_conv::renderer::setPixel(hConsole, screenBuffer, L'┼', x + i, y + 3, colour);
        }
    }
}

void DrawConveyorItem(
    HANDLE hConsole,
    cpp_conv::renderer::ScreenBuffer screenBuffer,
    wchar_t value,
    int x,
    int y,
    int iChannelIdx,
    int iChannelSlot,
    Direction direction,
    int colour,
    bool allowBackFill = false)
{
    switch (direction)
    {
    case Direction::Left:
        cpp_conv::renderer::setPixel(hConsole, screenBuffer, value, x + cpp_conv::c_conveyorChannelSlots - iChannelSlot, y + cpp_conv::c_conveyorChannels - iChannelIdx, colour, allowBackFill);
        break;
    case Direction::Up:
        cpp_conv::renderer::setPixel(hConsole, screenBuffer, value, x + cpp_conv::c_conveyorChannels - iChannelIdx, y + 1 + iChannelSlot, colour, allowBackFill);
        break;
    case Direction::Right:
        cpp_conv::renderer::setPixel(hConsole, screenBuffer, value, x + 1 + iChannelSlot, y + 1 + iChannelIdx, colour, allowBackFill);
        break;
    case Direction::Down:
        cpp_conv::renderer::setPixel(hConsole, screenBuffer, value, x + 1 + iChannelIdx, y + cpp_conv::c_conveyorChannelSlots - iChannelSlot, colour, allowBackFill);
        break;
    }
}

void DrawConveyorArrow(
    HANDLE hConsole,
    cpp_conv::renderer::ScreenBuffer screenBuffer,
    int x,
    int y,
    int iChannelIdx,
    int iChannelSlot,
    Direction direction,
    int colour)
{
    switch (direction)
    {
    case Direction::Left:
        DrawConveyorItem(hConsole, screenBuffer, L'←', x, y, iChannelIdx, iChannelSlot, direction, colour);
        break;
    case Direction::Up:
        DrawConveyorItem(hConsole, screenBuffer, L'↓', x, y, iChannelIdx, iChannelSlot, direction, colour);
        break;
    case Direction::Right:
        DrawConveyorItem(hConsole, screenBuffer, L'→', x, y, iChannelIdx, iChannelSlot, direction, colour);
        break;
    case Direction::Down:
        DrawConveyorItem(hConsole, screenBuffer, L'↑', x, y, iChannelIdx, iChannelSlot, direction, colour);
        break;
    }
}

cpp_conv::Conveyor::Channel::Channel()
{
    std::fill(std::begin(m_pItems), std::end(m_pItems), nullptr);
    std::fill(std::begin(m_pPendingItems), std::end(m_pPendingItems), nullptr);
}

cpp_conv::Conveyor::Conveyor(int32_t x, int32_t y, Direction direction, Item* pItem)
    : Entity(x, y, EntityKind::Conveyor)
    , m_direction(direction)
    , m_pSequenceId(0)
{
}

void cpp_conv::Conveyor::Tick(cpp_conv::grid::EntityGrid& grid)
{
}

void cpp_conv::Conveyor::Draw(HANDLE hConsole, cpp_conv::renderer::ScreenBuffer screenBuffer, cpp_conv::grid::EntityGrid& grid, int x, int y) const
{
    int colour = m_pSequenceId;

    int conveyorX = x * 3;
    int conveyorY = y * 3;

    DrawConveyorBorder(hConsole, screenBuffer, conveyorX, conveyorY, colour);

    for (int iChannelIdx = 0; iChannelIdx < c_conveyorChannels; ++iChannelIdx)
    {
        for (int iChannelSlot = 0; iChannelSlot < c_conveyorChannels; ++iChannelSlot)
        {
            if (m_pChannels[iChannelIdx].m_pItems[iChannelSlot] != nullptr)
            {
                DrawConveyorItem(
                    hConsole,
                    screenBuffer,
                    m_pChannels[iChannelIdx].m_pItems[iChannelSlot]->GetDisplayIcon(),
                    conveyorX,
                    conveyorY,
                    iChannelIdx,
                    iChannelSlot,
                    m_direction,
                    colour,
                    true);
            }
            else
            {
                DrawConveyorArrow(hConsole, screenBuffer, conveyorX, conveyorY, iChannelIdx, iChannelSlot, m_direction, colour);
            }
        }
    }
}
