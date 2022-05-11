#pragma once
#include "RenderContext.h"
#include "ScreenBuffer.h"
#include "WriteSurface.h"

namespace cpp_conv::renderer
{
    class SwapChain
    {
    public:
        SwapChain(RenderContext& kRenderContext, uint32_t uiWidth, uint32_t uiHeight);
        void Initialize(RenderContext& kRenderContext, ScreenBufferInitArgs& rArgs);
        void SwapAndPresent();
        WriteSurface& GetWriteSurface();

        bool RequiresResize(RenderContext& kRenderContext, int iWidth, int iHeight) const;
        void ResizeBuffers(RenderContext& kRenderContext, int iWidth, int iHeight);

    private:
        uint32_t m_uiCurrentBuffer;
        WriteSurface m_writeSurface;
        ScreenBuffer m_colourBuffers[2];
    };
}
