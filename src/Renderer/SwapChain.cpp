#include "SwapChain.h"

cpp_conv::renderer::SwapChain::SwapChain(RenderContext& kRenderContext, uint32_t uiWidth, uint32_t uiHeight)
    : m_uiCurrentBuffer(0)
    , m_writeSurface(kRenderContext, uiWidth, uiHeight, true)
    , m_colourBuffers
    {
        { m_writeSurface },
        { m_writeSurface }
    }
{
}

void cpp_conv::renderer::SwapChain::Initialize(RenderContext& kRenderContext, ScreenBufferInitArgs& rArgs)
{
    m_writeSurface.Initialize();
    for (ScreenBuffer& rScreenBuffer : m_colourBuffers)
    {
        rScreenBuffer.Initialize(kRenderContext, rArgs);
    }
}

void cpp_conv::renderer::SwapChain::SwapAndPresent()
{
    m_colourBuffers[m_uiCurrentBuffer].Present();
    m_uiCurrentBuffer = (m_uiCurrentBuffer + 1) % 2;
}

cpp_conv::renderer::WriteSurface& cpp_conv::renderer::SwapChain::GetWriteSurface()
{
    return m_writeSurface; 
}

bool cpp_conv::renderer::SwapChain::RequiresResize(RenderContext& kRenderContext, int iWidth, int iHeight) const
{
    return m_writeSurface.RequiresResize(kRenderContext, iWidth, iHeight);
}

void cpp_conv::renderer::SwapChain::ResizeBuffers(RenderContext& kRenderContext, int iWidth, int iHeight)
{
    m_writeSurface.Resize(kRenderContext, iWidth, iHeight);
    m_colourBuffers[0].RecreateBuffer(kRenderContext);
    m_colourBuffers[1].RecreateBuffer(kRenderContext);
}
