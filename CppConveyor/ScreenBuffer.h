#pragma once

#include <cstdint>
#include <vector>
#include <Windows.h>

namespace cpp_conv::renderer
{
	class WriteSurface;

	struct ScreenBufferInitArgs
	{
		CONSOLE_FONT_INFOEX m_surfaceFont;
	};

	class ScreenBuffer
	{
	public:
		ScreenBuffer(WriteSurface& rWriteSurface);
		~ScreenBuffer();

		void Initialize(ScreenBufferInitArgs& rArgs);

		void Present();

		void Shutdown();

		void RecreateBuffer();

	private:
		WriteSurface& m_rWriteSurface;
		HANDLE m_hBufferHandle;

		ScreenBufferInitArgs m_initArgs;
	};
}