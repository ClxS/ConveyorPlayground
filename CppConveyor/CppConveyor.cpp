#include <iostream>
#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <set>
#include <algorithm>
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include <Windows.h>

#include "vector_set.h"

#include "Entity.h"
#include "Grid.h"
#include "Sequence.h"
#include "Renderer.h"
#include "Simulator.h"

void DebugPopulate(cpp_conv::grid::EntityGrid& grid, int i)
{
    // Debug populate the grid
    if (i % 4 == 0)
    {
        cpp_conv::Conveyor* pConveyor = reinterpret_cast<cpp_conv::Conveyor*>(grid[6][20]);
        if (!pConveyor->m_pChannels[0].m_pItems[0])
        {
            pConveyor->m_pChannels[0].m_pItems[0] = new cpp_conv::Aluminium();
        }
    }

    /*if (i % 7 == 0)
    {
        cpp_conv::Conveyor* pConveyor = reinterpret_cast<cpp_conv::Conveyor*>(grid[6][20]);
        if (!pConveyor->m_pChannels[1].m_pItems[0])
        {
            pConveyor->m_pChannels[1].m_pItems[0] = new cpp_conv::Copper();
        }
    }*/

    //// Debug populate the grid
    //if (i % 8 == 0)
    //{
    //    cpp_conv::Conveyor* pConveyor = reinterpret_cast<cpp_conv::Conveyor*>(grid[2][0]);
    //    if (!pConveyor->m_pChannels[0].m_pItems[0])
    //    {
    //        pConveyor->m_pChannels[0].m_pItems[0] = new cpp_conv::Aluminium();
    //    }
    //}

    //if (i % 7 == 0)
    //{
    //    cpp_conv::Conveyor* pConveyor = reinterpret_cast<cpp_conv::Conveyor*>(grid[2][0]);
    //    if (!pConveyor->m_pChannels[1].m_pItems[0])
    //    {
    //        pConveyor->m_pChannels[1].m_pItems[0] = new cpp_conv::Copper();
    //    }
    //}
}

int main()
{
    cpp_conv::grid::EntityGrid grid;
    memset(&grid, 0, sizeof(grid));
    std::vector<cpp_conv::Conveyor*> conveyors;

    std::ifstream file("data.txt");

    std::string buf;
    int row = 0;
    while (getline(file, buf))
    {
        for(int col = 0; col < buf.size(); col++)
        {
            cpp_conv::Conveyor* pConveyor = nullptr;
            switch (buf[col])
            {
            case '>': pConveyor = new cpp_conv::Conveyor(col, row, Direction::Right); break;
            case '<': pConveyor = new cpp_conv::Conveyor(col, row, Direction::Left); break;
            case 'A': pConveyor = new cpp_conv::Conveyor(col, row, Direction::Right, new cpp_conv::Copper()); break;
            case 'D': pConveyor = new cpp_conv::Conveyor(col, row, Direction::Left, new cpp_conv::Copper()); break;
            case '^': pConveyor = new cpp_conv::Conveyor(col, row, Direction::Down); break;
            case '/': pConveyor = new cpp_conv::Conveyor(col, row, Direction::Up); break;
            }

            if (pConveyor)
            {
                grid[row][col] = pConveyor;
                conveyors.push_back(pConveyor);
            }
        }

        row++;
    }

    file.close();

    std::vector<cpp_conv::Sequence> sequences = cpp_conv::InitializeSequences(grid, conveyors);

    wchar_t screenBuffer[cpp_conv::renderer::c_screenHeight][cpp_conv::renderer::c_screenWidth];
    memset(screenBuffer, ' ', 10 * 10);

    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    cpp_conv::renderer::init(hConsole);
    for (int i = 0; i < 500; i++)
    {
        cpp_conv::simulation::Simulate(grid, sequences, conveyors);
        cpp_conv::renderer::PrintGrid(hConsole, screenBuffer, grid);

        std::this_thread::sleep_for(std::chrono::milliseconds(30));

        DebugPopulate(grid, i);        
    }

    CloseHandle(hConsole);
}
