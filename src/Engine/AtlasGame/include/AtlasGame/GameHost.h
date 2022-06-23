#pragma once
#include <algorithm>
#include <AtlasScene/SceneManager.h>

#include <SDL_syswm.h>
#include <AtlasAppHost/Application.h>
#include <AtlasAppHost/Main.h>
#include <AtlasGame/GameHost.h>
#include <AtlasResource/ResourceLoader.h>
#include <AtlasScene/SceneManager.h>
#include <bgfx/platform.h>

#include "AtlasRender/Renderer.h"
#include "Utility/FrameLimiter.h"

namespace atlas::game
{
    class GameImplementation
    {
    public:
        GameImplementation() = default;
        virtual ~GameImplementation() {}
        GameImplementation(const GameImplementation&) = delete;
        GameImplementation(const GameImplementation&&) = delete;
        GameImplementation operator=(const GameImplementation&) = delete;
        GameImplementation operator=(const GameImplementation&&) = delete;

        virtual void OnStartup() {}
        virtual void Tick()
        {
            m_SceneManager.Update();
        }

    protected:
        atlas::scene::SceneManager m_SceneManager;
    };

    template<typename TGameImplementation>
    class GameHost
    {
    public:
        struct Args
        {
            std::string m_GameName;
            int m_FrameRateCap = 60;
        };

        explicit GameHost(Args args = {})
            : m_GameArguments{std::move(args)}
        {
        }

        int Run()
        {
            if (!atlas::app_host::Application::Get().Initialise(m_GameArguments.m_GameName))
            {
                return -1;
            }

            logStartUp();
            srand(static_cast<unsigned>(time(nullptr)));
            auto [iWidth, iHeight] = atlas::app_host::Application::Get().GetAppDimensions();

            utility::FrameLimiter frameLimiter(m_GameArguments.m_FrameRateCap);
            frameLimiter.Start();

            auto& app = atlas::app_host::Application::Get();
#if !BX_PLATFORM_EMSCRIPTEN
            const auto& platform = app.GetPlatform();
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
            bgfx::renderFrame(); // single threaded mode
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

            render::init(args);
            m_Game.OnStartup();

            while(true)
            {
                bgfx::touch(0);

                app.Update();
                m_Game.Tick();

                atlas::render::sync();
                frameLimiter.Limit();
                frameLimiter.EndFrame();
            }

            return 0;
        }

    private:
        Args m_GameArguments;
        TGameImplementation m_Game;
    };
}
