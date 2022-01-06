#pragma once
#include "RenderContext.h"

namespace cpp_conv::renderer
{
	void setCell(RenderContext& kContext, wchar_t value, int x, int y, int colour, bool allowBackFill = false);
}