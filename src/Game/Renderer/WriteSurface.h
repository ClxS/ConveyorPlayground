#pragma once

#if defined(_CONSOLE)
#include "ConsoleWriteSurface.h"
#elif defined(_SDL)
#include "SDLWriteSurface.h"
#endif
