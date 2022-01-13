#include "SDLWriteSurface.h"
#include "SDL.h"
#include "SDLAppHost.h"

void cpp_conv::renderer::SDLWriteSurface::Clear()
{
    SDL_SetRenderDrawColor(cpp_conv::apphost::App.renderer, 96, 128, 255, 255);
    SDL_RenderClear(cpp_conv::apphost::App.renderer);
}
