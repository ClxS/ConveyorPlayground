#include "SDLScreenBuffer.h"
#include "SDL.h"
#include "SDLAppHost.h"

void cpp_conv::renderer::SDLScreenBuffer::Present()
{
    SDL_RenderPresent(cpp_conv::apphost::App.renderer);
}
