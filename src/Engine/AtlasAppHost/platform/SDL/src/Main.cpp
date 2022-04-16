#include "AtlasAppHostPCH.h"
#include "AtlasAppHost/Main.h"
#include "AtlasAppHost/Application.h"

// This method is here to keep the linker happy and stop it removing the SDL_main below...
void logStartUp()
{
}

#ifdef __cplusplus
extern "C" {
#endif
// ReSharper disable once CppInconsistentNaming
// ReSharper disable once CppParameterMayBeConst
int SDL_main(int argc, char* argv[])
{
    gameMain(argc, argv);

    SDL_Quit();
    return 0;
}
#ifdef __cplusplus
}
#endif
