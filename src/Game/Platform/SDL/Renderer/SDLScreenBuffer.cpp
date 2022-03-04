#include "SDLScreenBuffer.h"
#include "SDL.h"
#include "SDLAppHost.h"
#include "SDLWriteSurface.h"
#include "AtlasAppHost/Application.h"

void cpp_conv::renderer::SDLScreenBuffer::Present() const
{
    atlas::app_host::platform::PlatformApplication& platform = atlas::app_host::Application::Get().GetPlatform();
    SDL_RenderPresent(platform.GetSDLContext().m_Renderer);

    if (m_RWriteSurface.IsClearOnPresent())
    {
        m_RWriteSurface.Clear();
    }
}
