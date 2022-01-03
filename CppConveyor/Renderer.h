#pragma once

#include <Windows.h>

#include "Entity.h"
#include "Conveyor.h"
#include "Grid.h"

namespace cpp_conv
{
	namespace renderer
	{
        constexpr int c_screenWidth = 64;
        constexpr int c_screenHeight = 64;

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

        void PrintGrid(HANDLE hConsole, char screenBuffer[c_screenHeight][c_screenWidth], cpp_conv::grid::EntityGrid& grid)
        {
            for (int y = 0; y < grid.size(); y++)
            {
                for (int x = 0; x < grid[y].size(); x++)
                {
                    auto cell = grid[y][x];
                    if (cell == nullptr)
                    {
                        SetPixel(hConsole, screenBuffer, ' ', x, y);
                    }
                    else if (cell->m_eEntityKind == EntityKind::Conveyor)
                    {
                        const cpp_conv::Conveyor* pConveyor = reinterpret_cast<const cpp_conv::Conveyor*>(cell);
                        if (pConveyor->m_pItem != nullptr)
                        {
                            std::string strName = pConveyor->m_pItem->GetName();
                            if (strName == "Copper")
                            {
                                SetPixel(hConsole, screenBuffer, 'C', x, y);
                            }
                            else if (strName == "Tin")
                            {
                                SetPixel(hConsole, screenBuffer, 'T', x, y);
                            }
                            else if (strName == "Metal")
                            {
                                SetPixel(hConsole, screenBuffer, 'M', x, y);
                            }
                            else if (strName == "Aluminium")
                            {
                                SetPixel(hConsole, screenBuffer, 'A', x, y);
                            }
                            else
                            {
                                SetPixel(hConsole, screenBuffer, '*', x, y);
                            }
                        }
                        else
                        {
                            switch (pConveyor->m_direction)
                            {
                            case Direction::Left: SetPixel(hConsole, screenBuffer, '<', x, y); break;
                            case Direction::Up: SetPixel(hConsole, screenBuffer, '.', x, y); break;
                            case Direction::Right: SetPixel(hConsole, screenBuffer, '>', x, y); break;
                            case Direction::Down: SetPixel(hConsole, screenBuffer, '^', x, y); break;
                            }
                        }
                    }
                }
            }
        }
	}
}