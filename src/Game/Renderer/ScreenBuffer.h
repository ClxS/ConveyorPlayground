#pragma once

#if defined(_CONSOLE)
#include "ConsoleScreenBuffer.h"
#elif defined(_SDL)
#include "SDLScreenBuffer.h"
#endif

namespace cpp_conv::renderer
{
}
