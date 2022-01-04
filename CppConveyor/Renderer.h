#pragma once

#include <Windows.h>

#include "Entity.h"
#include "Conveyor.h"
#include "Grid.h"

namespace cpp_conv
{
	namespace renderer
	{
        constexpr int c_screenWidth = 256;
        constexpr int c_screenHeight = 256;

        void init(HANDLE hConsole)
        {
            CONSOLE_FONT_INFOEX cfi;
            cfi.cbSize = sizeof cfi;
            cfi.nFont = 0;
            cfi.dwFontSize.X = 0;
            cfi.dwFontSize.Y = 18;
            cfi.FontFamily = FF_DONTCARE;
            cfi.FontWeight = FW_HEAVY;

            wcscpy_s<32>(cfi.FaceName, L"Lucida Console");
            SetCurrentConsoleFontEx(hConsole, FALSE, &cfi);

            /*wcscpy(cfi.FaceName, L"Consolas");
            SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);*/
        }

        WORD GetColourAttribute(int colour)
        {
            switch (colour % 6)
            {
            case 0: return FOREGROUND_RED | FOREGROUND_INTENSITY;
            case 1: return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            case 2: return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            case 3: return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            case 4: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            case 5: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            }

            return FOREGROUND_RED | FOREGROUND_INTENSITY;
        }

        void SetPixel(HANDLE hConsole, wchar_t screenBuffer[c_screenHeight][c_screenWidth], wchar_t value, int x, int y,
            int colour)
        {
            if (screenBuffer[y][x] == value)
            {
                return;
            }

            screenBuffer[y][x] = value;

            COORD pos = { (SHORT)x, (SHORT)y };
            DWORD dwBytesWritten = 0;

            SetConsoleActiveScreenBuffer(hConsole);

            WORD attribute = GetColourAttribute(colour);
            WriteConsoleOutputAttribute(hConsole, &attribute, 1, pos, &dwBytesWritten);
            WriteConsoleOutputCharacterW(hConsole, &value, 1, pos, &dwBytesWritten);
        }

        void DrawConveyorBorder(HANDLE hConsole, wchar_t screenBuffer[c_screenHeight][c_screenWidth], int x, int y,
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
            wchar_t screenBuffer[c_screenHeight][c_screenWidth],
            wchar_t value,
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
                SetPixel(hConsole, screenBuffer, value, x + c_conveyorChannelSlots - iChannelSlot, y + c_conveyorChannels - iChannelIdx, colour);
                break;
            case Direction::Up:
                SetPixel(hConsole, screenBuffer, value, x + c_conveyorChannels - iChannelIdx, y + c_conveyorChannelSlots - iChannelSlot, colour);
                break;
            case Direction::Right:
                SetPixel(hConsole, screenBuffer, value, x + 1 + iChannelSlot, y + 1 + iChannelIdx, colour);
                break;
            case Direction::Down:
                SetPixel(hConsole, screenBuffer, value, x + 1 + iChannelIdx, y + 1 + iChannelSlot, colour);
                break;
            }
        }

        void DrawConveyorArrow(
            HANDLE hConsole, 
            wchar_t screenBuffer[c_screenHeight][c_screenWidth],
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

        void PrintGrid(HANDLE hConsole, wchar_t screenBuffer[c_screenHeight][c_screenWidth], cpp_conv::grid::EntityGrid& grid)
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

                        //SetPixel(hConsole, screenBuffer, 'C', x * 3, y * 3);

                        for (int iChannelIdx = 0; iChannelIdx < c_conveyorChannels; ++iChannelIdx)
                        {
                            for (int iChannelSlot = 0; iChannelSlot < c_conveyorChannels; ++iChannelSlot)
                            {
                                if (pConveyor->m_pChannels[iChannelIdx].m_pItems[iChannelSlot] != nullptr)
                                {
                                    const std::string& strName = pConveyor->m_pChannels[iChannelIdx].m_pItems[iChannelSlot]->GetName();
                                    if (strName == "Copper")
                                    {
                                        DrawConveyorItem(hConsole, screenBuffer, L'C', conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction, colour);
                                    }
                                    else if (strName == "Tin")
                                    {
                                        DrawConveyorItem(hConsole, screenBuffer, L'T', conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction, colour);
                                    }
                                    else if (strName == "Metal")
                                    {
                                        DrawConveyorItem(hConsole, screenBuffer, L'N', conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction, colour);
                                    }
                                    else if (strName == "Aluminium")
                                    {
                                        DrawConveyorItem(hConsole, screenBuffer, L'A', conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction, colour);
                                    }
                                    else
                                    {
                                        DrawConveyorItem(hConsole, screenBuffer, '*', conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction, colour);
                                    }
                                }
                                else
                                {
                                    DrawConveyorArrow(hConsole, screenBuffer, conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction, colour);
                                }
                            }
                        }
                    }
                }
            }
        }
	}
}