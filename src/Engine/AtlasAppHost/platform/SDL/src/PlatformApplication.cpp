#include "AtlasAppHostPCH.h"
#include "AtlasAppHost/PlatformApplication.h"

#include "SDL.h"

#define WINDOWED 1

bool atlas::app_host::platform::PlatformApplication::Initialise()
{
    int windowFlags;

    constexpr int rendererFlags = SDL_RENDERER_ACCELERATED;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return false;
    }

#if WINDOWED
    windowFlags = 0;
    m_Sdl.m_Window = SDL_CreateWindow("Cpp Conveyor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 800, windowFlags);
#else
    windowFlags = 0;
    m_Sdl.m_Window = SDL_CreateWindow("Cpp Conveyor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 2560, 1080, windowFlags);
    SDL_SetRelativeMouseMode(SDL_TRUE);
#endif

    if (!m_Sdl.m_Window)
    {
        printf("Failed to open %d x %d window: %s\n", 800, 600, SDL_GetError());
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");

    m_Sdl.m_Renderer = SDL_CreateRenderer(m_Sdl.m_Window, -1, rendererFlags);

    if (!m_Sdl.m_Renderer)
    {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

std::tuple<int, int> atlas::app_host::platform::PlatformApplication::GetAppDimensions() const
{
    int width, height;
    SDL_GetWindowSize(m_Sdl.m_Window, &width, &height);

    return std::make_tuple(width, height);
}
