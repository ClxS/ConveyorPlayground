#pragma once

#if defined(_CONSOLE)
#include "TextTileRenderHandler.h"
#elif defined(_SDL)
#include "SDLTileRenderHandler.h"
#endif
