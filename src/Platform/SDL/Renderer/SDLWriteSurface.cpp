#include "SDLWriteSurface.h"
#include "SDL.h"
#include "SDLAppHost.h"

void cpp_conv::renderer::SDLWriteSurface::Clear()
{
    SDL_SetRenderDrawColor(cpp_conv::apphost::App.renderer, 99, 132, 135, 255);
    SDL_RenderClear(cpp_conv::apphost::App.renderer);
}
