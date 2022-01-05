#pragma once

#include <Windows.h>
#include "Grid.h"
#include "ScreenBuffer.h"
#include "RenderContext.h"

namespace cpp_conv::renderer
{
	constexpr int c_gridScale = 3;

	void init(HANDLE& hConsole);
    void render(RenderContext& kContext);

	void setPixel(RenderContext& kContext, wchar_t value, int x, int y, int colour, bool allowBackFill = false);
}