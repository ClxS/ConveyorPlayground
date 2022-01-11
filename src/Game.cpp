#include "Game.h"
#include "AppHost.h"
#include "SwapChain.h"
#include "Renderer.h"
#include "Grid.h"
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

    ui::setContext(&kRenderContext);
    ui::panel("Game UI", ui::Align::Stretch);
        ui::panel("Right Panel", ui::Align::Right, 500);
            ui::text("CPP CONVEYORS!");
            ui::wrappedText("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam eget purus ut sem vulputate dictum. Vivamus consequat porta sagittis. Donec accumsan nisi et lorem porta, ut convallis ipsum rutrum. Pellentesque ullamcorper venenatis aliquam. Donec rhoncus dapibus magna sit amet laoreet. Nulla molestie sapien eget sem placerat, ac ultricies leo volutpat. Sed quis neque ac ante malesuada dignissim. Sed orci lectus, imperdiet non faucibus non, feugiat a massa. Morbi ullamcorper ex et purus maximus, a faucibus nibh elementum. Proin at sapien consectetur, faucibus lacus rhoncus, venenatis purus. Cras nec fringilla nisi. Cras eget faucibus velit, a elementum sem. Aenean non ligula mattis, blandit est quis, elementum lorem.");
        ui::endPanel();
        ui::panel("Footer", ui::Align::Bottom, 0, 70);
            ui::text(std::format("Current Pos: {}, {}", kSceneContext.m_player.m_x, kSceneContext.m_player.m_y));

            auto playerEntity = grid::SafeGetEntity(kSceneContext.m_grid, kSceneContext.m_player);
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
    AssetPtr<Map> map = resource_manager::loadAsset<Map>(registry::data::MapSimple);
    if (!map)
    {
        return;
    }

    std::vector<cpp_conv::Sequence> sequences = cpp_conv::InitializeSequences(map->GetGrid(), map->GetConveyors());
    cpp_conv::SceneContext kSceneContext =
    { 
        { 0, 0 },
        map->GetGrid(),
        sequences,
        map->GetConveyors(),
        map->GetOtherEntities(),
        { std::chrono::high_resolution_clock::now() }
    };

    cpp_conv::RenderContext kRenderContext =
    {
        { 0, 0, 0, 0 },
        nullptr,
        map->GetGrid(),
        1.0f
    };

    cpp_conv::renderer::SwapChain swapChain(iWidth, iHeight);
    cpp_conv::renderer::init(kRenderContext, swapChain);

    cpp_conv::FrameLimiter frameLimter(10);
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
            if (swapChain.RequiresResize(iNewWidth, iNewHeight))
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
        PROFILE(UpdateUI, updateUI(kSceneContext, kRenderContext));
        PROFILE(Present, swapChain.SwapAndPresent());

        PROFILE(FrameCapSleep, frameLimter.Limit());
        PROFILE(UpdatePersistence, cpp_conv::resources::resource_manager::updatePersistenceStore());
        frameLimter.EndFrame();
    }
}
