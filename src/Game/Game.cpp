#include "Game.h"
#include "AppHost.h"
#include "SwapChain.h"
#include "Renderer.h"
#include "EntityGrid.h"
#include "Sequence.h"
#include "SceneContext.h"
#include "RenderContext.h"
#include "Command.h"
#include "Input.h"
#include "Simulator.h"
#include "Renderer.h"
#include "Profiler.h"
#include "FrameLimiter.h"
#include "ResourceManager.h"
#include "Map.h"
#include "SelfRegistration.h"

#include "ItemRegistry.h"
#include "FactoryRegistry.h"
#include "Gui.h"

#include <tuple>
#include <vector>
#include <queue>
#include <chrono>
#include "WorldMap.h"
#include "Factory.h"
#include "AtlasAppHost/Application.h"
#include "AtlasAppHost/Main.h"

#undef max
#undef min

using namespace cpp_conv::resources;

int gameMain(int argc, char* argv[])
{
    logStartUp();
    srand(static_cast<unsigned>(time(nullptr)));
    auto [iWidth, iHeight] = atlas::app_host::Application::Get().GetAppDimensions();

    registration::processSelfRegistrations();

    cpp_conv::WorldMap worldMap;
    {
        const AssetPtr<Map> map = resource_manager::loadAssetUncached<Map>(registry::maps::c_simple);
        worldMap.Consume(map);
        worldMap.PopulateCorners();
    }

    std::vector<cpp_conv::Sequence*> sequences = initializeSequences(worldMap, worldMap.GetConveyors());
    cpp_conv::SceneContext kSceneContext =
    {
        worldMap,
        sequences,
        { std::chrono::high_resolution_clock::now() },
        {
            0,
            Direction::Right,
            true
        }
    };

    cpp_conv::RenderContext kRenderContext =
    {
        { 0.0f, 0.0f, 0.0f },
        0,
        { 0xFFFFFFFF },
        0,
        worldMap,
        nullptr,
        0.8f
    };

    cpp_conv::renderer::SwapChain swapChain(kRenderContext, iWidth, iHeight);
    init(kRenderContext, swapChain);

    cpp_conv::FrameLimiter frameLimiter(60);
    std::queue<cpp_conv::commands::CommandType> commands;

    cpp_conv::ui::initializeGuiSystem();


    frameLimiter.Start();
    float fCurrentZoom = kRenderContext.m_fZoom;
    while (true)
    {
        kRenderContext.m_uiDrawnItems = 0;

        PROFILE(Input, cpp_conv::input::receiveInput(kSceneContext, kRenderContext, commands));
        PROFILE(CommandProcess, cpp_conv::command::processCommands(kSceneContext, kRenderContext, commands));
        PROFILE(Simulation, cpp_conv::simulation::simulate(kSceneContext));
        PROFILE(ResizeSwap, [&]()
        {
            int iNewWidth;
            int iNewHeight;
            std::tie(iNewWidth, iNewHeight) = atlas::app_host::Application::Get().GetAppDimensions();
            if (swapChain.RequiresResize(kRenderContext, iNewWidth, iNewHeight))
            {
                swapChain.ResizeBuffers(kRenderContext, iNewWidth, iNewHeight);
            }
        }());

        PROFILE(Render, cpp_conv::renderer::render(kSceneContext, kRenderContext));
        PROFILE(DrawUI, cpp_conv::ui::drawUI(kSceneContext, kRenderContext));
        PROFILE(Present, swapChain.SwapAndPresent());

        PROFILE(FrameCapSleep, frameLimiter.Limit());
        PROFILE(UpdatePersistence, cpp_conv::resources::resource_manager::updatePersistenceStore());
        frameLimiter.EndFrame();
    }

    cpp_conv::ui::shutdown();
    return 0;
}

