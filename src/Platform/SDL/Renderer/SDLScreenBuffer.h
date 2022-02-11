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
        explicit SDLScreenBuffer(SDLWriteSurface& rWriteSurface)
            : m_RWriteSurface(rWriteSurface)
        {}
        ~SDLScreenBuffer() = default;

        void Initialize(RenderContext& kRenderContext, SDLScreenBufferInitArgs& rArgs) {}

        void Present() const;

        void Shutdown() {}

        void RecreateBuffer(RenderContext& kRenderContext) {}

    private:
        SDLWriteSurface& m_RWriteSurface;
        SDLScreenBufferInitArgs m_InitArgs;
    };

    using ScreenBuffer = SDLScreenBuffer;
    using ScreenBufferInitArgs = SDLScreenBufferInitArgs;
}
