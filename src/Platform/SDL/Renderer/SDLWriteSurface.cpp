#include "SDLWriteSurface.h"
#include "SDL.h"
#include "SDLAppHost.h"

void cpp_conv::renderer::SDLWriteSurface::Clear()
{
    SDL_SetRenderDrawColor(apphost::app.m_Renderer, 99, 132, 135, 255);
    SDL_RenderClear(apphost::app.m_Renderer);
}
