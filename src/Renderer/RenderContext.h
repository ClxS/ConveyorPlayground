#pragma once

#include "Renderer.h"
#include "WriteSurface.h"
#include "EntityGrid.h"
#include "Quad.h"
#include "WorldMap.h"

namespace cpp_conv
{
    struct RenderContext
    {
        Quad m_cameraQuad;
        cpp_conv::WorldMap& m_rMap;
        class cpp_conv::renderer::WriteSurface* m_surface;
        float m_fZoom;
    };
}
