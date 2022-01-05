#pragma once

#include <cstdint>
#include <vector>
#include <Windows.h>

namespace cpp_conv::renderer
{
	class WriteSurface;
	struct SurfaceInitArgs;

	class ScreenBuffer
	{
	public:
		ScreenBuffer(WriteSurface& rWriteSurface);
		~ScreenBuffer();

		void Initialize(SurfaceInitArgs& rArgs);

		void Present();

		void Shutdown();

	private:
		WriteSurface& m_rWriteSurface;
		HANDLE m_hBufferHandle;
	};
}