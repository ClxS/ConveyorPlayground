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

        void SetPixel(HANDLE hConsole, char screenBuffer[c_screenHeight][c_screenWidth], char value, int x, int y)
        {
            if (screenBuffer[y][x] == value)
            {
                return;
            }

            screenBuffer[y][x] = value;

            COORD pos = { (SHORT)x, (SHORT)y };
            DWORD dwBytesWritten = 0;

            SetConsoleActiveScreenBuffer(hConsole);
            WriteConsoleOutputCharacterA(hConsole, &value, 1, pos, &dwBytesWritten);
        }

        void DrawConveyorBorder(HANDLE hConsole, char screenBuffer[c_screenHeight][c_screenWidth], int x, int y)
        {
            for (int i = 0; i < 4; i++)
            {
                if (i != 0 && i != 3)
                {
                    SetPixel(hConsole, screenBuffer, '|', x, y + i);
                    SetPixel(hConsole, screenBuffer, '|', x + 3, y + i);
                }

                SetPixel(hConsole, screenBuffer, '=', x + i, y);
                SetPixel(hConsole, screenBuffer, '=', x + i, y + 3);
            }
        }

        void DrawConveyorItem(
            HANDLE hConsole, 
            char screenBuffer[c_screenHeight][c_screenWidth], 
            char value, 
            int x, 
            int y, 
            int iChannelIdx,
            int iChannelSlot,
            Direction direction)
        {
            switch (direction)
            {
            case Direction::Left:
                SetPixel(hConsole, screenBuffer, value, x + c_conveyorChannelSlots - iChannelSlot, y + c_conveyorChannels - iChannelIdx);
                break;
            case Direction::Up:
                SetPixel(hConsole, screenBuffer, value, x + c_conveyorChannels - iChannelIdx, y + c_conveyorChannelSlots - iChannelSlot);
                break;
            case Direction::Right:
                SetPixel(hConsole, screenBuffer, value, x + 1 + iChannelSlot, y + 1 + iChannelIdx);
                break;
            case Direction::Down:
                SetPixel(hConsole, screenBuffer, value, x + 1 + iChannelIdx, y + 1 + iChannelSlot);
                break;
            }
        }

        void DrawConveyorArrow(HANDLE hConsole, char screenBuffer[c_screenHeight][c_screenWidth],
            int x,
            int y,
            int iChannelIdx,
            int iChannelSlot,
            Direction direction)
        {
            switch (direction)
            {
            case Direction::Left:
                DrawConveyorItem(hConsole, screenBuffer, '<', x, y, iChannelIdx, iChannelSlot, direction);
                break;
            case Direction::Up:
                DrawConveyorItem(hConsole, screenBuffer, '/', x, y, iChannelIdx, iChannelSlot, direction);
                break;
            case Direction::Right:
                DrawConveyorItem(hConsole, screenBuffer, '>', x, y, iChannelIdx, iChannelSlot, direction);
                break;
            case Direction::Down:
                DrawConveyorItem(hConsole, screenBuffer, '^', x, y, iChannelIdx, iChannelSlot, direction);
                break;
            }
        }

        void PrintGrid(HANDLE hConsole, char screenBuffer[c_screenHeight][c_screenWidth], cpp_conv::grid::EntityGrid& grid)
        {
            for (int y = 0; y < grid.size(); y++)
            {
                for (int x = 0; x < grid[y].size(); x++)
                {
                    auto cell = grid[y][x];
                    if (cell == nullptr)
                    {
                        SetPixel(hConsole, screenBuffer, ' ', x * 4, y * 4);
                    }
                    else if (cell->m_eEntityKind == EntityKind::Conveyor)
                    {
                        const cpp_conv::Conveyor* pConveyor = reinterpret_cast<const cpp_conv::Conveyor*>(cell);

                        int conveyorX = x * 3;
                        int conveyorY = y * 3;
                        DrawConveyorBorder(hConsole, screenBuffer, conveyorX, conveyorY);

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
                                        DrawConveyorItem(hConsole, screenBuffer, 'C', conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction);
                                    }
                                    else if (strName == "Tin")
                                    {
                                        DrawConveyorItem(hConsole, screenBuffer, 'T', conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction);
                                    }
                                    else if (strName == "Metal")
                                    {
                                        DrawConveyorItem(hConsole, screenBuffer, 'N', conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction);
                                    }
                                    else if (strName == "Aluminium")
                                    {
                                        DrawConveyorItem(hConsole, screenBuffer, 'A', conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction);
                                    }
                                    else
                                    {
                                        DrawConveyorItem(hConsole, screenBuffer, '*', conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction);
                                    }
                                }
                                else
                                {
                                    DrawConveyorArrow(hConsole, screenBuffer, conveyorX, conveyorY, iChannelIdx, iChannelSlot, pConveyor->m_direction);
                                }
                            }
                        }

                        /**/
                    }
                }
            }
        }
	}
}