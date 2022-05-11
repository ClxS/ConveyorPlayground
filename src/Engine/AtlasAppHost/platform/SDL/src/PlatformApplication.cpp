#include "AtlasAppHostPCH.h"
#include "AtlasAppHost/PlatformApplication.h"

#include "SDL.h"
#include "backends/imgui_impl_sdl.h"

#define WINDOWED 1

bool atlas::app_host::platform::PlatformApplication::Initialise(std::string_view applicationName)
{
    int windowFlags;

    constexpr int rendererFlags = SDL_RENDERER_ACCELERATED;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    const std::string title {applicationName};
#if WINDOWED
    windowFlags = SDL_WINDOW_SHOWN;

    m_Sdl.m_Window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 800,
                                      windowFlags);
#else
    windowFlags = 0;
    m_Sdl.m_Window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 2560, 1080, windowFlags);
    SDL_SetRelativeMouseMode(SDL_TRUE);
#endif

    if (!m_Sdl.m_Window)
    {
        printf("Failed to open %d x %d window: %s\n", 800, 600, SDL_GetError());
        return false;
    }

    ImGui::CreateContext();
    return true;
}

std::tuple<int, int> atlas::app_host::platform::PlatformApplication::GetAppDimensions() const
{
    int width, height;
    SDL_GetWindowSize(m_Sdl.m_Window, &width, &height);

    return std::make_tuple(width, height);
}

void atlas::app_host::platform::PlatformApplication::Update()
{
    SDL_Event currentEvent;
    while(SDL_PollEvent(&currentEvent) != 0)
    {
        ImGui_ImplSDL2_ProcessEvent(&currentEvent);
        switch (currentEvent.type)
        {
        case SDL_QUIT:
            exit(0);
            break;
        default:
            break;
        }

        // TODO
        if (currentEvent.type == SDL_KEYDOWN)
        {
            int i = 0;
            i++;
        }
    }

    SDL_PumpEvents();
}
