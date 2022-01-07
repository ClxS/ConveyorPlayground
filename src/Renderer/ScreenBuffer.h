#pragma once

#if defined(_CONSOLE)
#include "ConsoleScreenBuffer.h"
#endif

namespace cpp_conv::renderer
{
    using ScreenBuffer = ConsoleScreenBuffer;
}