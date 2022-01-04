#include "Renderer.h"

WORD GetColourAttribute(int colour, bool allowBackFill)
{
    if (allowBackFill)
    {
        switch (colour % 6)
        {
        case 0: return FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_RED;
        case 1: return FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_GREEN;
        case 2: return FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
        case 3: return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE;
        case 4: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE;
        case 5: return FOREGROUND_GREEN | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_GREEN;
        }
    }
    else
    {
        switch (colour % 6)
        {
        case 0: return FOREGROUND_RED | FOREGROUND_INTENSITY;
        case 1: return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case 2: return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        case 3: return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        case 4: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        case 5: return FOREGROUND_GREEN | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        }
    }

    return FOREGROUND_RED | FOREGROUND_INTENSITY;
}

void SetPixel(HANDLE hConsole, cpp_conv::renderer::ScreenBuffer screenBuffer, wchar_t value, int x, int y,
    int colour, bool allowBackFill = false)
{
    if (screenBuffer[y][x] == value)
    {
        return;
    }

    screenBuffer[y][x] = value;

    COORD pos = { (SHORT)x, (SHORT)y };
    DWORD dwBytesWritten = 0;

    SetConsoleActiveScreenBuffer(hConsole);

    WORD attribute = GetColourAttribute(colour, allowBackFill);
    WriteConsoleOutputAttribute(hConsole, &attribute, 1, pos, &dwBytesWritten);
    WriteConsoleOutputCharacterW(hConsole, &value, 1, pos, &dwBytesWritten);
}

void DrawConveyorBorder(HANDLE hConsole, cpp_conv::renderer::ScreenBuffer screenBuffer, int x, int y,
    int colour)
{
    for (int i = 0; i < 4; i++)
    {
        if (i != 0 && i != 3)
        {
            SetPixel(hConsole, screenBuffer, L'│', x, y + i, colour);
            SetPixel(hConsole, screenBuffer, L'│', x + 3, y + i, colour);
        }

        if (i != 0 && i != 3)
        {
            SetPixel(hConsole, screenBuffer, L'─', x + i, y, colour);
            SetPixel(hConsole, screenBuffer, L'─', x + i, y + 3, colour);
        }

        if (i == 0 || i == 3)
        {
            SetPixel(hConsole, screenBuffer, L'┼', x + i, y, colour);
            SetPixel(hConsole, screenBuffer, L'┼', x + i, y + 3, colour);
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
        SetPixel(hConsole, screenBuffer, value, x + cpp_conv::c_conveyorChannelSlots - iChannelSlot, y + cpp_conv::c_conveyorChannels - iChannelIdx, colour, allowBackFill);
        break;
    case Direction::Up:
        SetPixel(hConsole, screenBuffer, value, x + cpp_conv::c_conveyorChannels - iChannelIdx, y + 1 + iChannelSlot, colour, allowBackFill);
        break;
    case Direction::Right:
        SetPixel(hConsole, screenBuffer, value, x + 1 + iChannelSlot, y + 1 + iChannelIdx, colour, allowBackFill);
        break;
    case Direction::Down:
        SetPixel(hConsole, screenBuffer, value, x + 1 + iChannelIdx, y + cpp_conv::c_conveyorChannelSlots - iChannelSlot, colour, allowBackFill);
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

void cpp_conv::renderer::init(HANDLE& hConsole)
{
    hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof cfi;
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 18;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_HEAVY;

    wcscpy_s<32>(cfi.FaceName, L"Lucida Console");
    SetCurrentConsoleFontEx(hConsole, FALSE, &cfi);
}

void cpp_conv::renderer::render(HANDLE hConsole, wchar_t screenBuffer[c_screenHeight][c_screenWidth], cpp_conv::grid::EntityGrid& grid)
{
    for (int y = 0; y < grid.size(); y++)
    {
        for (int x = 0; x < grid[y].size(); x++)
        {
            auto cell = grid[y][x];
            if (cell == nullptr)
            {
                SetPixel(hConsole, screenBuffer, L' ', x * 4, y * 4, 0);
            }
            else if (cell->m_eEntityKind == EntityKind::Conveyor)
            {
                const cpp_conv::Conveyor* pConveyor = reinterpret_cast<const cpp_conv::Conveyor*>(cell);

                int colour = pConveyor->m_pSequenceId;

                int conveyorX = x * 3;
                int conveyorY = y * 3;

                DrawConveyorBorder(hConsole, screenBuffer, conveyorX, conveyorY, colour);

                for (int iChannelIdx = 0; iChannelIdx < c_conveyorChannels; ++iChannelIdx)
                {
                    for (int iChannelSlot = 0; iChannelSlot < c_conveyorChannels; ++iChannelSlot)
                    {
                        if (pConveyor->m_pChannels[iChannelIdx].m_pItems[iChannelSlot] != nullptr)
                        {
                            DrawConveyorItem(
                                hConsole,
                                screenBuffer,
                                pConveyor->m_pChannels[iChannelIdx].m_pItems[iChannelSlot]->GetDisplayIcon(),
                                conveyorX,
                                conveyorY,
                                iChannelIdx,
                                iChannelSlot,
                                pConveyor->m_direction,
                                colour,
                                true);
                        }
                        else
                        {
                            DrawConveyorArrow(hConsole, screenBuffer, conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction, colour);
                        }
                    }
                }
            }
            else if (cell->m_eEntityKind == EntityKind::Producer)
            {
                cpp_conv::Producer* pProducer = reinterpret_cast<cpp_conv::Producer*>(cell);

                for (int conveyorY = y * 3 + 1; conveyorY < y * 3 + 3; conveyorY++)
                {
                    for (int conveyorX = x * 3 + 1; conveyorX < x * 3 + 3; conveyorX++)
                    {
                        switch (pProducer->GetDirection())
                        {
                        case Direction::Left:
                            SetPixel(hConsole, screenBuffer, L'←', conveyorX, conveyorY, 0, true);
                            break;
                        case Direction::Up:
                            SetPixel(hConsole, screenBuffer, L'↓', conveyorX, conveyorY, 0, true);
                            break;
                        case Direction::Right:
                            SetPixel(hConsole, screenBuffer, L'→', conveyorX, conveyorY, 0, true);
                            break;
                        case Direction::Down:
                            SetPixel(hConsole, screenBuffer, L'↑', conveyorX, conveyorY, 0, true);
                            break;
                        }
                    }
                }
            }
        }
    }
}
