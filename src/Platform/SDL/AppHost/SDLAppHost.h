#pragma once

struct SDL_Window;
struct SDL_Renderer;

namespace cpp_conv::apphost
{
    struct SdlContext
    {
        SDL_Renderer* renderer;
        SDL_Window* window;
    };

    extern SdlContext App;
}
