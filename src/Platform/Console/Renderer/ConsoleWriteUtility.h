#pragma once
#include "RenderContext.h"

namespace cpp_conv::renderer
{
    void setCell(RenderContext& kContext, wchar_t value, int x, int y, WORD colour, bool bSuppressPan = false);
    WORD getWin32Colour(cpp_conv::Colour colour);
}
