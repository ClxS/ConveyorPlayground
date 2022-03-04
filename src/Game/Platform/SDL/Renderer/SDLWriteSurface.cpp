#include "SDLWriteSurface.h"
#include "SDL.h"
#include "SDLAppHost.h"
#include "AtlasAppHost/Application.h"

void cpp_conv::renderer::SDLWriteSurface::Clear()
{
    atlas::app_host::platform::PlatformApplication& platform = atlas::app_host::Application::Get().GetPlatform();
    SDL_SetRenderDrawColor(platform.GetSDLContext().m_Renderer, 99, 132, 135, 255);
    SDL_RenderClear(platform.GetSDLContext().m_Renderer);
}
