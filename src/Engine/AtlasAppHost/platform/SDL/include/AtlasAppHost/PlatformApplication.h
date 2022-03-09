#pragma once

#include <tuple>

// ReSharper disable once CppInconsistentNaming
struct SDL_Window;

// ReSharper disable once CppInconsistentNaming
struct SDL_Renderer;

namespace atlas::app_host::platform
{
    class PlatformApplication
    {
    public:
        struct SdlContext
        {
            SDL_Renderer* m_Renderer = nullptr;
            SDL_Window* m_Window = nullptr;
        };

        bool Initialise();

        [[nodiscard]] std::tuple<int, int> GetAppDimensions() const;
        [[nodiscard]] const SdlContext& GetSDLContext() const { return m_Sdl; }
        [[nodiscard]] SdlContext& GetSDLContext() { return m_Sdl; }

    private:
        SdlContext m_Sdl {};
    };
}