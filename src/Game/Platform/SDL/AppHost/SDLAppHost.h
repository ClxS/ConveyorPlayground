#pragma once

// ReSharper disable once CppInconsistentNaming
struct SDL_Window;

// ReSharper disable once CppInconsistentNaming
struct SDL_Renderer;

namespace cpp_conv::apphost
{
    struct SdlContext
    {
        SDL_Renderer* m_Renderer;
        SDL_Window* m_Window;
    };

    extern SdlContext app;
}
