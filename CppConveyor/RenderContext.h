#pragma once

#include "Renderer.h"
#include "WriteSurface.h"
#include "Grid.h"

namespace cpp_conv
{
	struct RenderContext
	{
		int cameraOffsetX;
		int cameraOffsetY;
		cpp_conv::renderer::WriteSurface& m_surface;
		cpp_conv::grid::EntityGrid& m_grid;
	};
}