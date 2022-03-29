#pragma once

#include "EntityGrid.h"
#include "Quad.h"
#include "Renderer.h"
#include "WorldMap.h"
#include "WriteSurface.h"

namespace cpp_conv
{
    struct RenderContext
    {
        Vector3F m_CameraPosition;
        int32_t m_iCurrentLayer;
        Colour m_LayerColour;

        uint32_t m_uiCurrentDrawPass;
        WorldMap& m_rMap;
        class renderer::WriteSurface* m_surface;
        float m_fZoom;

        uint32_t m_uiDrawnItems;
    };
}
