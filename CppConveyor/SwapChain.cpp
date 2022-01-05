#include "SwapChain.h"

cpp_conv::renderer::SwapChain::SwapChain(uint32_t uiWidth, uint32_t uiHeight)
	: m_uiCurrentBuffer(0)
	, m_writeSurface(uiWidth, uiHeight, true)
	, m_colourBuffers
	{
		{ m_writeSurface },
		{ m_writeSurface }
	}
{
}

void cpp_conv::renderer::SwapChain::Initialize(SurfaceInitArgs& rArgs)
{
	m_writeSurface.Initialize();
	for (ScreenBuffer& rScreenBuffer : m_colourBuffers)
	{
		rScreenBuffer.Initialize(rArgs);
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
