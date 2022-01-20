#include "Input.h"
#include "AppHost.h"
#include "Profiler.h"
#include "CommandType.h"
#include "SDL_events.h"

#include <backends/imgui_impl_sdl.h>

void cpp_conv::input::receiveInput(std::queue<cpp_conv::commands::CommandType>& commands)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            exit(0);
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
                case SDLK_PAGEUP:
                    commands.push(cpp_conv::commands::CommandType::MoveFloorUp);
                    break;
                case SDLK_PAGEDOWN:
                    commands.push(cpp_conv::commands::CommandType::MoveFloorDown);
                    break;
                case SDLK_ESCAPE:
                    exit(0);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
        }
    }
}
