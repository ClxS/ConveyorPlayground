#pragma once
#include "RenderContext.h"

namespace cpp_conv::renderer
{
    std::tuple<uint32_t, uint32_t> getConsoleSpaceDimensions(uint32_t uiWidth, uint32_t uiHeight, float scale);

    void setCell(RenderContext& kContext, wchar_t value, int x, int y, WORD colour, bool bSuppressPan = false);
    WORD getWin32Colour(cpp_conv::Colour colour);
}
