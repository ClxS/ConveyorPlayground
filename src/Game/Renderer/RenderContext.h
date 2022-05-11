#pragma once

#include "Renderer.h"
#include "WriteSurface.h"

namespace cpp_conv
{
    struct RenderContext
    {
        int32_t m_iCurrentLayer;
        Colour m_LayerColour;

        uint32_t m_uiCurrentDrawPass;
        class renderer::WriteSurface* m_surface;
        float m_fZoom;

        uint32_t m_uiDrawnItems;
    };
}
