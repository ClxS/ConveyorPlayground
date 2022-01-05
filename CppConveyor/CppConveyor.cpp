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
#include "SceneContext.h"
#include "RenderContext.h"
#include "SwapChain.h"

int main()
{
    srand(time(NULL));

    cpp_conv::grid::EntityGrid grid;
    memset(&grid, 0, sizeof(grid));
    std::vector<cpp_conv::Conveyor*> conveyors;
    std::vector<cpp_conv::Entity*> vOtherEntities;

    cpp_conv::file_reader::readFile("data.txt", grid, conveyors, vOtherEntities);
    std::vector<cpp_conv::Sequence> sequences = cpp_conv::InitializeSequences(grid, conveyors);

    cpp_conv::renderer::SwapChain swapChain(256, 256);
    cpp_conv::renderer::init(swapChain);

    cpp_conv::SceneContext kSceneContext = { grid, sequences, conveyors, vOtherEntities };
    cpp_conv::RenderContext kRenderContext = { swapChain.GetWriteSurface(), grid};

    std::chrono::high_resolution_clock clock = {};
    std::chrono::steady_clock::time_point startTime = clock.now();

    uint32_t frameCounter = 0;
    while(true)
    {
        cpp_conv::simulation::simulate(kSceneContext);
        cpp_conv::renderer::render(kRenderContext);
        swapChain.SwapAndPresent();
        frameCounter++;

        std::chrono::steady_clock::time_point currentTime = clock.now();
        if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime) >= std::chrono::seconds{ 1 })
        {
            startTime = currentTime;

            OutputDebugStringA((std::string("\nFps: ") + std::to_string(frameCounter)).c_str());
            frameCounter = 0;
        }
    }
}
