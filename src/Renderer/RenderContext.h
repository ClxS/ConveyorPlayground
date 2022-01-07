#pragma once

#include "Renderer.h"
#include "WriteSurface.h"
#include "Grid.h"
#include "Quad.h"

namespace cpp_conv
{
	struct RenderContext
	{
		Quad m_cameraQuad;
		cpp_conv::renderer::WriteSurface& m_surface;
		cpp_conv::grid::EntityGrid& m_grid;
	};
}