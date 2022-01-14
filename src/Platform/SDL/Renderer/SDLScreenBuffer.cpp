#include "SDLScreenBuffer.h"
#include "SDL.h"
#include "SDLAppHost.h"
#include "SDLWriteSurface.h"

void cpp_conv::renderer::SDLScreenBuffer::Present()
{
    SDL_RenderPresent(cpp_conv::apphost::App.renderer);

    if (m_rWriteSurface.IsClearOnPresent())
    {
        m_rWriteSurface.Clear();
    }
}
