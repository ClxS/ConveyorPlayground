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
        Vector2F m_CameraPosition;
        int32_t m_iCurrentLayer;
        Colour m_LayerColour;

        uint32_t m_uiCurrentDrawPass;
        WorldMap& m_rMap;
        class renderer::WriteSurface* m_surface;
        float m_fZoom;

        uint32_t m_uiDrawnItems;
    };
}
