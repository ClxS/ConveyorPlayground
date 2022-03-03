#include "SDLScreenBuffer.h"
#include "SDL.h"
#include "SDLAppHost.h"
#include "SDLWriteSurface.h"

void cpp_conv::renderer::SDLScreenBuffer::Present() const
{
    SDL_RenderPresent(apphost::app.m_Renderer);

    if (m_RWriteSurface.IsClearOnPresent())
    {
        m_RWriteSurface.Clear();
    }
}
