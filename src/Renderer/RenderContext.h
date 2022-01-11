#pragma once

#include "Renderer.h"
#include "WriteSurface.h"
#include "EntityGrid.h"
#include "Quad.h"

namespace cpp_conv
{
    struct RenderContext
    {
        Quad m_cameraQuad;
        class cpp_conv::renderer::WriteSurface* m_surface;
        cpp_conv::grid::EntityGrid& m_grid;
        float m_fZoom;
    };
}
