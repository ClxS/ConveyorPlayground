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

#undef max
#undef min

using namespace cpp_conv::resources;

namespace
{
    void updateCamera(cpp_conv::SceneContext& kContext, cpp_conv::RenderContext& kRenderContext)
    {
        constexpr int c_iPadding = 3 * cpp_conv::renderer::c_gridScale;
        constexpr int c_iPlayerSize = cpp_conv::renderer::c_gridScale;
        int playerX = kContext.m_player.GetX() * cpp_conv::renderer::c_gridScale;
        int playerY = kContext.m_player.GetY() * cpp_conv::renderer::c_gridScale;

        if (playerX < kRenderContext.m_cameraQuad.GetLeft() + c_iPadding)
        {
            kRenderContext.m_cameraQuad.m_x = playerX - c_iPadding;
        }

        if (playerY < kRenderContext.m_cameraQuad.GetTop() + (c_iPadding + c_iPlayerSize))
        {
            kRenderContext.m_cameraQuad.m_y = playerY - c_iPadding;
        }

        if (playerX > (kRenderContext.m_cameraQuad.GetRight() - (c_iPadding + c_iPlayerSize)))
        {
            kRenderContext.m_cameraQuad.m_x = (playerX + c_iPadding + c_iPlayerSize) - kRenderContext.m_cameraQuad.m_uiWidth;
        }

        if (playerY > (kRenderContext.m_cameraQuad.GetBottom() - (c_iPadding + c_iPlayerSize)))
        {
            kRenderContext.m_cameraQuad.m_y = (playerY + c_iPadding + c_iPlayerSize) - kRenderContext.m_cameraQuad.m_uiHeight;
        }

        kRenderContext.m_cameraQuad.m_x = std::max(kRenderContext.m_cameraQuad.m_x, 0);
        kRenderContext.m_cameraQuad.m_y = std::max(kRenderContext.m_cameraQuad.m_y, 0);
    }
}

void cpp_conv::game::run()
{
    srand((unsigned int)time(NULL));
     
    int iWidth, iHeight;
    std::tie(iWidth, iHeight) = cpp_conv::apphost::getAppDimensions();

    cpp_conv::resources::registration::processSelfRegistrations();

    WorldMap worldMap;
    {
        AssetPtr<Map> map = resource_manager::loadAssetUncached<Map>(registry::data::MapSimple);
        if (!map)
        {
            return;
        }

        worldMap.Consume(map);
    }

    std::vector<cpp_conv::Sequence> sequences = cpp_conv::InitializeSequences(worldMap, worldMap.GetConveyors());
    cpp_conv::SceneContext kSceneContext =
    { 
        {},
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
        { 0, 0, 0, 0 },
        0,
        0,
        worldMap,
        nullptr,
        0.5f
    };

    cpp_conv::renderer::SwapChain swapChain(kRenderContext, iWidth, iHeight);
    cpp_conv::renderer::init(kRenderContext, swapChain);

    cpp_conv::FrameLimiter frameLimter(1);
    std::queue<cpp_conv::commands::CommandType> commands;

    cpp_conv::ui::initializeGuiSystem();

    frameLimter.Start();
    float fCurrentZoom = kRenderContext.m_fZoom;
    while (true) 
    {
        PROFILE(Input, cpp_conv::input::receiveInput(commands));
        PROFILE(CommandProcess, cpp_conv::command::processCommands(kSceneContext, kRenderContext, commands));
        PROFILE(Simulation, cpp_conv::simulation::simulate(kSceneContext));
        PROFILE(ResizeSwap, [&]() {
            int iNewWidth;
            int iNewHeight;
            std::tie(iNewWidth, iNewHeight) = cpp_conv::apphost::getAppDimensions();
            if (swapChain.RequiresResize(kRenderContext, iNewWidth, iNewHeight))
            {
                swapChain.ResizeBuffers(kRenderContext, iNewWidth, iNewHeight);
                kRenderContext.m_cameraQuad.m_uiWidth = swapChain.GetWriteSurface().GetWidth();
                kRenderContext.m_cameraQuad.m_uiHeight = swapChain.GetWriteSurface().GetHeight();
            }
            else if (fCurrentZoom != kRenderContext.m_fZoom)
            {
                swapChain.ResizeBuffers(kRenderContext, iNewWidth, iNewHeight);
                fCurrentZoom = kRenderContext.m_fZoom;
            }
            }()); 

        PROFILE(UpdateCamera, updateCamera(kSceneContext, kRenderContext));

        PROFILE(Render, cpp_conv::renderer::render(kSceneContext, kRenderContext));
        PROFILE(DrawUI, cpp_conv::ui::drawUI(kSceneContext, kRenderContext));
        PROFILE(Present, swapChain.SwapAndPresent());

        PROFILE(FrameCapSleep, frameLimter.Limit());
        PROFILE(UpdatePersistence, cpp_conv::resources::resource_manager::updatePersistenceStore());
        frameLimter.EndFrame();
    }

    cpp_conv::ui::shutdown();
}
