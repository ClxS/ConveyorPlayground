#pragma once

#if !defined(_SDL)
#error Invalid Include
#endif

#include <cstdint>

namespace cpp_conv { struct RenderContext; }
namespace cpp_conv::renderer
{
    class SDLWriteSurface
    {
    public:
        SDLWriteSurface(cpp_conv::RenderContext& kRenderContext,
                        uint16_t uiWidth,
                        uint16_t uiHeight,
                        bool bClearOnPresent)
            : m_uiWidth(uiWidth)
            , m_uiHeight(uiHeight)
            , m_bClearOnPresent(true)
        { 
        }

        void Initialize() {}
        void Clear();

        uint16_t GetWidth() const { return m_uiWidth; }
        uint16_t GetHeight() const { return m_uiHeight; }
        bool IsClearOnPresent() const { return m_bClearOnPresent; }

        void Resize(RenderContext& kRenderContext, int iWidth, int iHeight) {}
        bool RequiresResize(RenderContext& kRenderContext, int iWidth, int iHeight) const { return false; }

    private:
        uint16_t m_uiWidth;
        uint16_t m_uiHeight;
        bool m_bClearOnPresent;
    };

    using WriteSurface = SDLWriteSurface;
}
