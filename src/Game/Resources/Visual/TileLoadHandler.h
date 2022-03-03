#pragma once

#if defined(_CONSOLE)
#include "TextTileLoadHandler.h"
#elif defined(_SDL)
#include "SDLTileLoadHandler.h"
#endif
