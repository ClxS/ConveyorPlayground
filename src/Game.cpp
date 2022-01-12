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

using namespace cpp_conv::resources;

namespace
{
    void updateCamera(cpp_conv::SceneContext& kContext, cpp_conv::RenderContext& kRenderContext)
    {
        constexpr int c_iPadding = 3 * cpp_conv::renderer::c_gridScale;
        constexpr int c_iPlayerSize = cpp_conv::renderer::c_gridScale;
        int playerX = kContext.m_player.m_x * cpp_conv::renderer::c_gridScale;
        int playerY = kContext.m_player.m_y * cpp_conv::renderer::c_gridScale;

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

        kRenderContext.m_cameraQuad.m_x = max(kRenderContext.m_cameraQuad.m_x, 0);
        kRenderContext.m_cameraQuad.m_y = max(kRenderContext.m_cameraQuad.m_y, 0);
    }
}

void updateUI(cpp_conv::SceneContext& kSceneContext, cpp_conv::RenderContext& kRenderContext)
{
    using namespace cpp_conv;
    constexpr const char* c_entityNames[] =
    {
        "Conveyor",
        "Inserter",
        "Junction",
        "Producer",
        "Stairs",
        "Storage",
        "Underground",
    };

    static_assert((int32_t)EntityKind::MAX == sizeof(c_entityNames) / sizeof(const char*), "Table mismatch");

    ui::setContext(&kRenderContext);
    ui::panel("Game UI", ui::Align::Stretch);
        ui::panel("Right Panel", ui::Align::Right, 700);
            ui::text("Item Type (NUM 1-9)", { 0x0000FF00 });
            for (int32_t i = 0; i < (int32_t)EntityKind::MAX; ++i)
            {
                ui::text(
                    std::format(
                        "{} {}. {}",
                        kSceneContext.m_uiContext.m_selected == i ? ">" : " ",
                        i + 1,
                        c_entityNames[i]),
                    (kSceneContext.m_uiContext.m_selected == i ? Colour { 0x000000FF } : Colour{ 0x00FFFFFF }));
            }
            ui::text("Rotation (R to rotate)", { 0x0000FF00 });
            switch (kSceneContext.m_uiContext.m_rotation)
            {
            case Direction::Up:
                ui::text(L"   v", {0x00FFFF00});
                break;
            case Direction::Down:
                ui::text(L"   ^", { 0x00FFFF00 });
                break;
            case Direction::Left:
                ui::text(L"   <", { 0x00FFFF00 });
                break;
            case Direction::Right:
                ui::text(L"   >", { 0x00FFFF00 });
                break;
            default:
                break;
            }

            if (kSceneContext.m_uiContext.m_selected == (int)EntityKind::Stairs)
            {
                ui::text("Stairs go up? (T)", { 0x0000FF00 });
                ui::text(kSceneContext.m_uiContext.m_bModifier ? L"   true" : L"   false", { 0x00FFFF00 });

            }
        ui::endPanel();
        ui::panel("Footer", ui::Align::Bottom, 0, 3_Lines, false);
            ui::text(std::format("Current Pos: {}, {}. Floor: {}", kSceneContext.m_player.m_x, kSceneContext.m_player.m_y, kSceneContext.m_player.m_depth));
            auto playerEntity = kSceneContext.m_rMap.GetEntity(kSceneContext.m_player);
            if (playerEntity)
            {
                ui::text(std::format("Item: {} - {}", playerEntity->GetName(), playerEntity->GetDescription()));
            }
            else
            {
                ui::text("Item: Nothing", { 0x00FF0000 });
            }
        ui::endPanel();
    ui::endPanel();
    ui::setContext(nullptr);
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
        { 0, 0 },
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
        worldMap,
        nullptr,
        0.3f
    };

    cpp_conv::renderer::SwapChain swapChain(kRenderContext, iWidth, iHeight);
    cpp_conv::renderer::init(kRenderContext, swapChain);

    cpp_conv::FrameLimiter frameLimter(500);
    std::queue<cpp_conv::commands::CommandType> commands;

    cpp_conv::ui::initializeGuiSystem(1920, 1080);

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
        //PROFILE(UpdateUI, updateUI(kSceneContext, kRenderContext));
        PROFILE(Present, swapChain.SwapAndPresent());

        PROFILE(FrameCapSleep, frameLimter.Limit());
        PROFILE(UpdatePersistence, cpp_conv::resources::resource_manager::updatePersistenceStore());
        frameLimter.EndFrame();
    }
}
