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
        SDLWriteSurface(RenderContext& kRenderContext,
                        const uint16_t uiWidth,
                        const uint16_t uiHeight,
                        const bool bClearOnPresent)
            : m_uiWidth(uiWidth)
            , m_uiHeight(uiHeight)
            , m_bClearOnPresent(bClearOnPresent)
        {
        }

        void Initialize() {}
        void Clear();

        [[nodiscard]] uint16_t GetWidth() const { return m_uiWidth; }
        [[nodiscard]] uint16_t GetHeight() const { return m_uiHeight; }
        [[nodiscard]] bool IsClearOnPresent() const { return m_bClearOnPresent; }

        void Resize(RenderContext& kRenderContext, int iWidth, int iHeight) {}
        bool RequiresResize(RenderContext& kRenderContext, int iWidth, int iHeight) const { return false; }

    private:
        uint16_t m_uiWidth;
        uint16_t m_uiHeight;
        bool m_bClearOnPresent;
    };

    using WriteSurface = SDLWriteSurface;
}
