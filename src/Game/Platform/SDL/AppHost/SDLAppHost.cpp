#include "SDLAppHost.h"
#include <tuple>
#include "AppHost.h"
#include "SDL.h"

std::tuple<int, int> cpp_conv::apphost::getCursorPosition()
{
    int mouseX, mouseY;
    uint32_t buttons = SDL_GetMouseState(&mouseX, &mouseY);
    return std::make_tuple(mouseX, mouseY);
}
