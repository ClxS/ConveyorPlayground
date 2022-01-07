#pragma once

#include <cstdint>
#include <vector>
#include <Windows.h>

namespace cpp_conv::renderer
{
    class ConsoleWriteSurface;

    struct ConsoleScreenBufferInitArgs
    {
        CONSOLE_FONT_INFOEX m_surfaceFont;
    };

    class ConsoleScreenBuffer
    {
    public:
        ConsoleScreenBuffer(ConsoleWriteSurface& rWriteSurface);
        ~ConsoleScreenBuffer();

        void Initialize(ConsoleScreenBufferInitArgs& rArgs);

        void Present();

        void Shutdown();

        void RecreateBuffer();

    private:
        ConsoleWriteSurface& m_rWriteSurface;
        HANDLE m_hBufferHandle;

        ConsoleScreenBufferInitArgs m_initArgs;
    };

    using ScreenBuffer = ConsoleScreenBuffer;
    using ScreenBufferInitArgs = ConsoleScreenBufferInitArgs;
}