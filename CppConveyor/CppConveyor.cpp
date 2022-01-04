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
#include "Producer.h"

int main()
{
    cpp_conv::grid::EntityGrid grid;
    memset(&grid, 0, sizeof(grid));
    std::vector<cpp_conv::Conveyor*> conveyors;
    std::vector<cpp_conv::Producer*> producers;

    std::ifstream file("data.txt");

    std::string buf;
    int row = 0;
    while (getline(file, buf))
    {
        for(int col = 0; col < buf.size(); col++)
        {
            cpp_conv::Entity* pEntity = nullptr;
            switch (buf[col])
            {
            case '>': pEntity = new cpp_conv::Conveyor(col, row, Direction::Right); break;
            case '<': pEntity = new cpp_conv::Conveyor(col, row, Direction::Left); break;
            case '^': pEntity = new cpp_conv::Conveyor(col, row, Direction::Down); break;
            case '/': pEntity = new cpp_conv::Conveyor(col, row, Direction::Up); break;
            case 'A':
            {
                cpp_conv::Producer* pProducer = new cpp_conv::Producer(col, row, Direction::Right, new cpp_conv::Copper(), 5);
                producers.push_back(pProducer);
                pEntity = pProducer;
                break;
            }
            case 'D':
            {
                cpp_conv::Producer* pProducer = new cpp_conv::Producer(col, row, Direction::Left, new cpp_conv::Copper(), 5);
                producers.push_back(pProducer);
                pEntity = pProducer;
                break;
            }
            }

            if (pEntity)
            {
                grid[row][col] = pEntity;

                if (pEntity->m_eEntityKind == EntityKind::Conveyor)
                {
                    conveyors.push_back(reinterpret_cast<cpp_conv::Conveyor*>(pEntity));
                }
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
    while(true)
    {
        cpp_conv::simulation::simulate(grid, sequences, conveyors, producers);
        cpp_conv::renderer::render(hConsole, screenBuffer, grid);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    CloseHandle(hConsole);
}
