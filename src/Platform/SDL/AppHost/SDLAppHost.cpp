#include <tuple>
#include "AppHost.h"
#include "Game.h"
#include "SDL.h"
#include "SDLAppHost.h"

#define WINDOWED 1

namespace cpp_conv::apphost
{
    cpp_conv::apphost::SdlContext App;
}

std::tuple<int, int> cpp_conv::apphost::getAppDimensions()
{
    int width, height;
    SDL_GetWindowSize(cpp_conv::apphost::App.window, &width, &height);

    return std::make_tuple(width, height);
}

void createWindow()
{
    int rendererFlags, windowFlags;

    rendererFlags = SDL_RENDERER_ACCELERATED;


    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    #if WINDOWED
    windowFlags = 0;
    cpp_conv::apphost::App.window = SDL_CreateWindow("Cpp Conveyor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 2560, 1080, windowFlags);
    #else
    windowFlags = 0;
    cpp_conv::apphost::App.window = SDL_CreateWindow("Cpp Conveyor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 1000, windowFlags);
    #endif

    if (!cpp_conv::apphost::App.window)
    {
        printf("Failed to open %d x %d window: %s\n", 800, 600, SDL_GetError());
        exit(1);
    }

    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");

    cpp_conv::apphost::App.renderer = SDL_CreateRenderer(cpp_conv::apphost::App.window, -1, rendererFlags);

    if (!cpp_conv::apphost::App.renderer)
    {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        exit(1);
    }
} 

#ifdef __cplusplus
extern "C"
{
#endif
    int SDL_main(int argc, char* argv[])
    {
        createWindow();
        cpp_conv::game::run();

        SDL_Quit();
        return 0;
    }
#ifdef __cplusplus
}
#endif
