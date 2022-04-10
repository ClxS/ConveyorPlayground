#include "Game.h"
#include "FrameLimiter.h"
#include "RenderContext.h"
#include "Renderer.h"
#include "SwapChain.h"

#include <chrono>
#include <tuple>

#include "ConveyorComponent.h"
#include "ConveyorDefinition.h"
#include "ConveyorRegistry.h"
#include "DescriptionComponent.h"
#include "DirectionComponent.h"
#include "FactoryComponent.h"
#include "FactoryRegistry.h"
#include "GameMapLoadInterstitialScene.h"
#include "InserterRegistry.h"
#include "ItemRegistry.h"
#include "MapLoadHandler.h"
#include "NameComponent.h"
#include "PositionComponent.h"
#include "RecipeDefinition.h"
#include "RecipeRegistry.h"
#include "SDLTileLoadHandler.h"
#include "SDL_syswm.h"
#include "SequenceComponent.h"
#include "SpriteLayerComponent.h"
#include "WorldEntityInformationComponent.h"
#include "../Engine/AtlasRender/include/AtlasRender/Renderer.h"
#include "AtlasAppHost/Application.h"
#include "AtlasAppHost/Main.h"
#include "AtlasResource/ResourceLoader.h"
#include "AtlasScene/SceneManager.h"
#include "bgfx/platform.h"

#undef max
#undef min

using namespace cpp_conv::resources;

cpp_conv::RenderContext* g_renderContext;

void registerComponents()
{
    using namespace atlas::resource;
    using namespace atlas::scene;
    using namespace cpp_conv::components;
    ComponentRegistry::RegisterComponent<NameComponent>();
    ComponentRegistry::RegisterComponent<DescriptionComponent>();
    ComponentRegistry::RegisterComponent<ConveyorComponent>();
    ComponentRegistry::RegisterComponent<IndividuallyProcessableConveyorComponent>();
    ComponentRegistry::RegisterComponent<DirectionComponent>();
    ComponentRegistry::RegisterComponent<FactoryComponent>();
    ComponentRegistry::RegisterComponent<PositionComponent>();
    ComponentRegistry::RegisterComponent<SequenceComponent>();
    ComponentRegistry::RegisterComponent<SpriteLayerComponent<1>>();
    ComponentRegistry::RegisterComponent<SpriteLayerComponent<2>>();
    ComponentRegistry::RegisterComponent<SpriteLayerComponent<3>>();
    ComponentRegistry::RegisterComponent<WorldEntityInformationComponent>();
}

void registerAssetBundles()
{
    using namespace atlas::resource;
    ResourceLoader::RegisterBundle<registry::CoreBundle>();
}

void registerTypeHandlers()
{
    using namespace atlas::resource;
    ResourceLoader::RegisterTypeHandler<Map>(mapAssetHandler);
    ResourceLoader::RegisterTypeHandler<cpp_conv::ConveyorDefinition>(conveyorAssetHandler);
    ResourceLoader::RegisterTypeHandler<cpp_conv::FactoryDefinition>(factoryAssetHandler);
    ResourceLoader::RegisterTypeHandler<cpp_conv::InserterDefinition>(inserterAssetHandler);
    ResourceLoader::RegisterTypeHandler<cpp_conv::ItemDefinition>(itemAssetHandler);
    ResourceLoader::RegisterTypeHandler<cpp_conv::RecipeDefinition>(recipeAssetHandler);
    ResourceLoader::RegisterTypeHandler<TileAsset>(cpp_conv::textTileLoadHandler);
}

void loadDataAssets()
{
    loadConveyors();
    loadFactories();
    loadInserters();
    loadItems();
    loadRecipes();
}

int gameMain(int argc, char* argv[])
{
    using namespace atlas::resource;
    using namespace atlas::scene;

    logStartUp();
    srand(static_cast<unsigned>(time(nullptr)));
    auto [iWidth, iHeight] = atlas::app_host::Application::Get().GetAppDimensions();

    registerComponents();
    registerTypeHandlers();
    registerAssetBundles();
    loadDataAssets();

    SceneManager sceneManager;
    sceneManager.TransitionTo<cpp_conv::GameMapLoadInterstitialScene>(
        ResourceLoader::CreateBundleRegistryId<registry::CoreBundle>(registry::core_bundle::maps::c_bigmap));

    cpp_conv::FrameLimiter frameLimiter(120);
    frameLimiter.Start();

    // TODO REMOVE THIS
    cpp_conv::RenderContext kRenderContext =
    {
        0,
        {0xFFFFFFFF},
        0,
        nullptr,
        0.8f
    };
    g_renderContext = &kRenderContext;

    cpp_conv::renderer::SwapChain swapChain(kRenderContext, iWidth, iHeight);

#if !BX_PLATFORM_EMSCRIPTEN
    const auto& platform = atlas::app_host::Application::Get().GetPlatform();
    const auto dimensions = platform.GetAppDimensions();
    auto window = atlas::app_host::Application::Get().GetPlatform().GetSDLContext().m_Window;
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    if (!SDL_GetWindowWMInfo(window, &wmi)) {
        printf(
            "SDL_SysWMinfo could not be retrieved. SDL_Error: %s\n",
            SDL_GetError());
        return 1;
    }
    #endif // !BX_PLATFORM_EMSCRIPTEN

    atlas::render::RendererInitArgs args;
    args.m_Width = std::get<0>(dimensions);
    args.m_Height = std::get<1>(dimensions);

#if BX_PLATFORM_WINDOWS
    args.m_WindowHandle = wmi.info.win.window;
#elif BX_PLATFORM_OSX
    args.m_WindowHandle = wmi.info.cocoa.window;
#elif BX_PLATFORM_LINUX
    args.m_WindowHandle = (void*)(uintptr_t)wmi.info.x11.window;
#elif BX_PLATFORM_EMSCRIPTEN
    args.m_WindowHandle = (void*)"#canvas";
#endif

    init(args);

    while (true)
    {
        kRenderContext.m_uiDrawnItems = 0;

        sceneManager.Update();

        atlas::render::sync();
        frameLimiter.Limit();

        swapChain.SwapAndPresent();
        frameLimiter.EndFrame();
    }

    /*cpp_conv::WorldMap worldMap;
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

    cpp_conv::ui::shutdown();*/
    return 0;
}
