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
            case 'A': pConveyor = new cpp_conv::Conveyor(col, row, Direction::Left, new cpp_conv::Copper()); break;
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

    char screenBuffer[cpp_conv::renderer::c_screenHeight][cpp_conv::renderer::c_screenWidth];
    memset(screenBuffer, ' ', 10 * 10);

    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    for (int i = 0; i < 200; i++)
    {
        cpp_conv::simulation::Simulate(grid, sequences, conveyors);
        cpp_conv::renderer::PrintGrid(hConsole, screenBuffer, grid);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Debug populate the grid
        if (i % 5 == 0)
        {
            cpp_conv::Conveyor* pConveyor = reinterpret_cast<cpp_conv::Conveyor*>(grid[6][20]);
            if (!pConveyor->m_pItem)
            {
                pConveyor->m_pItem = new cpp_conv::Aluminium();
            }
        }
    }

    CloseHandle(hConsole);
}
