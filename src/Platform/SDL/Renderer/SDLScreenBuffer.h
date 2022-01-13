#pragma once

#if !defined(_SDL)
#error Invalid Include
#endif

namespace cpp_conv { struct RenderContext; }
namespace cpp_conv::renderer
{
    class SDLWriteSurface;

    struct SDLScreenBufferInitArgs
    {
    };

    class SDLScreenBuffer
    {
    public:
        SDLScreenBuffer(SDLWriteSurface& rWriteSurface)
            : m_rWriteSurface(rWriteSurface)
        {}
        ~SDLScreenBuffer() {}

        void Initialize(RenderContext& kRenderContext, SDLScreenBufferInitArgs& rArgs) {}

        void Present() {}

        void Shutdown() {}

        void RecreateBuffer(RenderContext& kRenderContext) {}

    private:
        SDLWriteSurface& m_rWriteSurface;
        SDLScreenBufferInitArgs m_initArgs;
    };

    using ScreenBuffer = SDLScreenBuffer;
    using ScreenBufferInitArgs = SDLScreenBufferInitArgs;
}
