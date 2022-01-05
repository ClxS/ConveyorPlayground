#pragma once

#include "Renderer.h"
#include "WriteSurface.h"

namespace cpp_conv
{
	struct RenderContext
	{
		cpp_conv::renderer::WriteSurface& m_surface;
		cpp_conv::grid::EntityGrid& m_grid;
	};
}