#include <iostream>
#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <set>
#include <algorithm>
#include <chrono>
#include <thread>
#include <Windows.h>

#include "vector_set.h"

#include "Entity.h"
#include "Grid.h"
#include "Sequence.h"
#include "Renderer.h"
#include "Simulator.h"
#include "Producer.h"
#include "FileReader.h"

int main()
{
    srand(time(NULL));

    cpp_conv::grid::EntityGrid grid;
    memset(&grid, 0, sizeof(grid));
    std::vector<cpp_conv::Conveyor*> conveyors;
    std::vector<cpp_conv::Producer*> producers;

    cpp_conv::file_reader::readFile("data.txt", grid, conveyors, producers);
    std::vector<cpp_conv::Sequence> sequences = cpp_conv::InitializeSequences(grid, conveyors);

    wchar_t screenBuffer[cpp_conv::renderer::c_screenHeight][cpp_conv::renderer::c_screenWidth];
    memset(screenBuffer, ' ', 10 * 10);

    HANDLE hConsole;
    cpp_conv::renderer::init(hConsole);
    while(true)
    {
        cpp_conv::simulation::simulate(grid, sequences, conveyors, producers);
        cpp_conv::renderer::render(hConsole, screenBuffer, grid);

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    CloseHandle(hConsole);
}
