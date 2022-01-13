#pragma once

#if defined(_CONSOLE)
#include "TextTileAsset.h"
#elif defined(_SDL)
#include "SDLTileAsset.h"
#endif
