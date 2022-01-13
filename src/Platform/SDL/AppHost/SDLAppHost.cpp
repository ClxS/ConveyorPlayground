#include <tuple>
#include "AppHost.h"
#include "Game.h"

std::tuple<int, int> cpp_conv::apphost::getAppDimensions()
{
    return std::make_tuple(1, 1);
}

#ifdef __cplusplus
extern "C"
{
#endif
    int SDL_main(int argc, char* argv[])
    {
        cpp_conv::game::run();
        return 0;
    }
#ifdef __cplusplus
}
#endif
