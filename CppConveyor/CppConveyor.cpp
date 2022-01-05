#include <iostream>
#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <set>
#include <algorithm>
#include <chrono>
#include <thread>
#include <format>
#include <mutex>
#include <queue>
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
#include "Input.h"
#include "Command.h"

#define STR_COMBINE_DIRECT(X,Y) X##Y
#define STR_COMBINE2(X,Y) STR_COMBINE_DIRECT(X,Y)
#define PROFILE(VAR, INSTRUCTION)\
    auto STR_COMBINE2(start, __LINE__) = clock.now();\
    INSTRUCTION;\
    auto STR_COMBINE2(end, __LINE__) = clock.now();\
    VAR += STR_COMBINE2(end, __LINE__) - STR_COMBINE2(start, __LINE__);

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 2)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

std::tuple<int, int> getConsoleDimensions()
{
    HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r); //stores the console's current dimensions

    // current window size
    short winWidth = r.right - r.left + 1;
    short winHeight = r.bottom - r.top + 1;

    return std::make_tuple(winWidth - 100, winHeight - 100);
}

int main()
{
    srand(time(NULL));
    
    cpp_conv::grid::EntityGrid grid;
    memset(&grid, 0, sizeof(grid));
    std::vector<cpp_conv::Conveyor*> conveyors;
    std::vector<cpp_conv::Entity*> vOtherEntities;

    cpp_conv::file_reader::readFile("data.txt", grid, conveyors, vOtherEntities);
    std::vector<cpp_conv::Sequence> sequences = cpp_conv::InitializeSequences(grid, conveyors);

    int width, height;
    std::tie(width, height) = getConsoleDimensions();

    cpp_conv::renderer::SwapChain swapChain(width, height);
    cpp_conv::renderer::init(swapChain);

    std::chrono::high_resolution_clock clock = {};
    std::chrono::steady_clock::time_point startTime = clock.now();
    cpp_conv::SceneContext kSceneContext = { { 0, 0 }, grid, sequences, conveyors, vOtherEntities, { startTime }};
    cpp_conv::RenderContext kRenderContext = { 0, 0, swapChain.GetWriteSurface(), grid};


    uint32_t frameCounter = 0;

    constexpr auto targetFrameTime = std::chrono::duration<int64_t, std::ratio<1, 2000>>(1);
    auto nextFrame = clock.now() + targetFrameTime;
    std::chrono::steady_clock::time_point lastFrame = {};
    std::chrono::nanoseconds inputRecieveTime = {};
    std::chrono::nanoseconds commandProcessTime = {};
    std::chrono::nanoseconds simulationTime = {};
    std::chrono::nanoseconds renderTime = {};
    std::chrono::nanoseconds sleepTime = {};
    std::chrono::nanoseconds presentTime = {};
    std::queue<cpp_conv::commands::InputCommand> commands;

    while(true)
    {
        frameCounter++;

        PROFILE(inputRecieveTime, cpp_conv::input::receiveInput(commands));
        PROFILE(commandProcessTime, cpp_conv::command::processCommands(kSceneContext, commands));
        PROFILE(simulationTime, cpp_conv::simulation::simulate(kSceneContext));

        std::tie(width, height) = getConsoleDimensions();
        if (swapChain.RequiresResize(width, height))
        {
            swapChain.ResizeBuffers(width, height);
        }

        PROFILE(renderTime, cpp_conv::renderer::render(kSceneContext, kRenderContext));
        PROFILE(sleepTime, std::this_thread::sleep_until(nextFrame));
        PROFILE(presentTime, swapChain.SwapAndPresent());
        HWND console = GetConsoleWindow();
        SetConsoleMode(console, 0);

        nextFrame += targetFrameTime;
        lastFrame = clock.now();
        if (std::chrono::duration_cast<std::chrono::seconds>(lastFrame - startTime) >= std::chrono::seconds{ 5 })
        {
            startTime = lastFrame;

            auto totalTime = inputRecieveTime + simulationTime + renderTime + sleepTime + presentTime;
            std::string strMsg = std::format("\n\nFPS: {}\nInput: {} ({}%)\nCommand: {} ({}%)\nSimulation: {} ({}%)\nRender: {} ({}%)\nSleep: {} ({}%)\nPresent: {} ({}%)",
                std::to_string(frameCounter / 5),
                to_string_with_precision(std::chrono::duration_cast<std::chrono::milliseconds>(inputRecieveTime / frameCounter)),
                to_string_with_precision(((double)inputRecieveTime.count() / (double)totalTime.count()) * 100),
                to_string_with_precision(std::chrono::duration_cast<std::chrono::milliseconds>(commandProcessTime / frameCounter)),
                to_string_with_precision(((double)commandProcessTime.count() / (double)totalTime.count()) * 100),
                to_string_with_precision(std::chrono::duration_cast<std::chrono::milliseconds>(simulationTime / frameCounter)),
                to_string_with_precision(((double)simulationTime.count() / (double)totalTime.count()) * 100),
                to_string_with_precision(std::chrono::duration_cast<std::chrono::milliseconds>(renderTime / frameCounter)),
                to_string_with_precision(((double)renderTime.count() / (double)totalTime.count()) * 100),
                to_string_with_precision(std::chrono::duration_cast<std::chrono::milliseconds>(sleepTime / frameCounter)),
                to_string_with_precision(((double)sleepTime.count() / (double)totalTime.count()) * 100),
                to_string_with_precision(std::chrono::duration_cast<std::chrono::milliseconds>(presentTime / frameCounter)),
                to_string_with_precision(((double)presentTime.count() / (double)totalTime.count()) * 100)
                );

            OutputDebugStringA(strMsg.c_str());
            OutputDebugStringA(
                std::format("\nW: {}, H: {}", 
                std::to_string(swapChain.GetWriteSurface().GetWidth()), 
                std::to_string(swapChain.GetWriteSurface().GetHeight())).c_str());
        }

        lastFrame = clock.now();
    }
}
