#pragma once
#include "WriteSurface.h"
#include "ScreenBuffer.h"

namespace cpp_conv::renderer
{
	struct SurfaceInitArgs
	{
		CONSOLE_FONT_INFOEX m_surfaceFont;
	};

	class SwapChain
	{
	public:	
		SwapChain(uint32_t uiWidth, uint32_t uiHeight);
		void Initialize(SurfaceInitArgs& rArgs);
		void SwapAndPresent();
		WriteSurface& GetWriteSurface();

	private:
		uint32_t m_uiCurrentBuffer;
		WriteSurface m_writeSurface;
		ScreenBuffer m_colourBuffers[2];
	};
}