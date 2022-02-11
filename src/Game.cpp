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

#undef max
#undef min

using namespace cpp_conv::resources;

namespace
{
    void updateCamera(cpp_conv::SceneContext& kContext, cpp_conv::RenderContext& kRenderContext)
    {
        /*constexpr int c_iPadding = 3 * cpp_conv::renderer::c_gridScale;
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
        kRenderContext.m_cameraQuad.m_y = std::max(kRenderContext.m_cameraQuad.m_y, 0);*/
    }
}

void CreateMillionTileMap(cpp_conv::WorldMap& worldMap)
{
    worldMap.PlaceEntity({ 0, 0, 0 }, new cpp_conv::Factory({ (int32_t)0, 0, 0 }, Direction::Right, cpp_conv::FactoryId::FromStringId("FACTORY_COPPER_MINE")));
    int count = 0;
    for (int y = 1; y < 31 * 64; y += 2)
    {
        if (y == 1)
        {
            worldMap.PlaceEntity({ 3, y, 0 }, new cpp_conv::Conveyor({ 3, y, 0 }, { 1, 1, 1 }, Direction::Right));
            worldMap.PlaceEntity({ 3, y + 1, 0 }, new cpp_conv::Conveyor({ 3, y + 1, 0 }, { 1, 1, 1 }, Direction::Up));
            count += 2;
        }
        else
        {
            worldMap.PlaceEntity({ 3, y, 0 }, new cpp_conv::Conveyor({ 3, y, 0 }, { 1, 1, 1 }, Direction::Right));
            worldMap.PlaceEntity({ 3, y + 1, 0 }, new cpp_conv::Conveyor({ 3, y + 1, 0 }, { 1, 1, 1 }, Direction::Up));
            count += 2;
        }

        const int width = 250;
        //const int width = 31 * 64 - 1;
        for (int x = 4; x < width; x++)
        {
            worldMap.PlaceEntity({ x, y, 0 }, new cpp_conv::Conveyor({ x, y, 0 }, { 1, 1, 1 }, Direction::Right));
            worldMap.PlaceEntity({ x, y + 1, 0 }, new cpp_conv::Conveyor({ x, y + 1, 0 }, { 1, 1, 1 }, Direction::Left));
            count += 2;

            if (count >= 1000)
            {
                return;
            }
        }

        worldMap.PlaceEntity({ width, y, 0 }, new cpp_conv::Conveyor({ width, y, 0 }, { 1, 1, 1 }, Direction::Up));
        worldMap.PlaceEntity({ width, y + 1, 0 }, new cpp_conv::Conveyor({ width, y + 1, 0 }, { 1, 1, 1 }, Direction::Left));
        count += 2;
    }
}

void cpp_conv::game::run()
{
    srand((unsigned int)time(NULL));

    int iWidth, iHeight;
    std::tie(iWidth, iHeight) = apphost::getAppDimensions();

    registration::processSelfRegistrations();

    WorldMap worldMap;
    {
        //CreateMillionTileMap(worldMap);

        const AssetPtr<Map> map = resource_manager::loadAssetUncached<Map>(registry::data::MapSimple);
        worldMap.Consume(map);

        /*map = resource_manager::loadAssetUncached<Map>(registry::data::MapSimple1);
        for (auto& pEntity : map->GetConveyors()) { pEntity->m_position.SetZ(1); }
        for (auto& pEntity : map->GetOtherEntities()) { pEntity->m_position.SetZ(1); }
        worldMap.Consume(map);

        map = resource_manager::loadAssetUncached<Map>(registry::data::MapSimple2);
        for (auto& pEntity : map->GetConveyors()) { pEntity->m_position.SetZ(2); }
        for (auto& pEntity : map->GetOtherEntities()) { pEntity->m_position.SetZ(2); }
        worldMap.Consume(map);

        map = resource_manager::loadAssetUncached<Map>(registry::data::MapSimple3);
        for (auto& pEntity : map->GetConveyors()) { pEntity->m_position.SetZ(3); }
        for (auto& pEntity : map->GetOtherEntities()) { pEntity->m_position.SetZ(3); }
        worldMap.Consume(map);*/

        worldMap.PopulateCorners();
    }

    std::vector<Sequence*> sequences = initializeSequences(worldMap, worldMap.GetConveyors());
    SceneContext kSceneContext =
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

    RenderContext kRenderContext =
    {
        { 0, 0 },
        0,
        { 0xFFFFFFFF },
        0,
        worldMap,
        nullptr,
        0.8f
    };

    renderer::SwapChain swapChain(kRenderContext, iWidth, iHeight);
    init(kRenderContext, swapChain);

    FrameLimiter frameLimter(1);
    std::queue<commands::CommandType> commands;

    ui::initializeGuiSystem();


    frameLimter.Start();
    float fCurrentZoom = kRenderContext.m_fZoom;
    while (true) 
    {
        kRenderContext.m_uiDrawnItems = 0;

        PROFILE(Input, cpp_conv::input::receiveInput(kSceneContext, kRenderContext, commands));
        PROFILE(CommandProcess, cpp_conv::command::processCommands(kSceneContext, kRenderContext, commands));
        PROFILE(Simulation, cpp_conv::simulation::simulate(kSceneContext));
        PROFILE(ResizeSwap, [&]() {
            int iNewWidth;
            int iNewHeight;
            std::tie(iNewWidth, iNewHeight) = cpp_conv::apphost::getAppDimensions();
            if (swapChain.RequiresResize(kRenderContext, iNewWidth, iNewHeight))
            {
                swapChain.ResizeBuffers(kRenderContext, iNewWidth, iNewHeight);
                /*kRenderContext.m_cameraQuad.m_uiWidth = swapChain.GetWriteSurface().GetWidth();
                kRenderContext.m_cameraQuad.m_uiHeight = swapChain.GetWriteSurface().GetHeight();*/
            }
            }()); 

        PROFILE(UpdateCamera, updateCamera(kSceneContext, kRenderContext));

        PROFILE(Render, cpp_conv::renderer::render(kSceneContext, kRenderContext));
        //PROFILE(DrawUI, cpp_conv::ui::drawUI(kSceneContext, kRenderContext));
        PROFILE(Present, swapChain.SwapAndPresent());

        PROFILE(FrameCapSleep, frameLimter.Limit());
        PROFILE(UpdatePersistence, cpp_conv::resources::resource_manager::updatePersistenceStore());
        frameLimter.EndFrame();
    }

    ui::shutdown();
}

