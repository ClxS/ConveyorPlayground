#pragma once

#include "Renderer.h"
#include "ScreenBuffer.h"

namespace cpp_conv
{
	struct RenderContext
	{
		HANDLE m_hConsole;

		cpp_conv::renderer::ScreenBuffer& m_screenBuffer;

		cpp_conv::grid::EntityGrid& m_grid;
	};
}