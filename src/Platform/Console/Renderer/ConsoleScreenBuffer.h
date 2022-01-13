#pragma once

#include <cstdint>
#include <vector>
#include <Windows.h>
#include "RenderContext.h"

#if !defined(_CONSOLE)
#error Invalid Include
#endif

namespace cpp_conv::renderer
{
    class ConsoleWriteSurface;

    struct ConsoleScreenBufferInitArgs
    {
    };

    class ConsoleScreenBuffer
    {
    public:
        ConsoleScreenBuffer(ConsoleWriteSurface& rWriteSurface);
        ~ConsoleScreenBuffer();

        void Initialize(RenderContext& kRenderContext, ConsoleScreenBufferInitArgs& rArgs);

        void Present();

        void Shutdown();

        void RecreateBuffer(RenderContext& kRenderContext);

    private:
        ConsoleWriteSurface& m_rWriteSurface;
        HANDLE m_hBufferHandle;

        ConsoleScreenBufferInitArgs m_initArgs;
    };

    using ScreenBuffer = ConsoleScreenBuffer;
    using ScreenBufferInitArgs = ConsoleScreenBufferInitArgs;
}
