#include "Input.h"
#include "AppHost.h"
#include "Profiler.h"
#include "CommandType.h"
#include "SDL_events.h"

void cpp_conv::input::receiveInput(std::queue<cpp_conv::commands::CommandType>& commands)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            exit(0);
            break;

        default:
            break;
        }
    }
}
