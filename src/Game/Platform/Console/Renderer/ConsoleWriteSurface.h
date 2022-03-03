#pragma once

#include <cstdint>
#include <vector>
#include <Windows.h>

#if !defined(_CONSOLE)
#error Invalid Include
#endif

namespace cpp_conv { struct RenderContext; }

namespace cpp_conv::renderer
{
    class ConsoleWriteSurface
    {
    public:
        ConsoleWriteSurface(cpp_conv::RenderContext& kRenderContext, uint16_t uiWidth, uint16_t uiHeight, bool bClearOnPresent);

        void Initialize();

        void Clear();

        std::vector<CHAR_INFO>& GetData() { return m_chiBuffer; }
        [[nodiscard]] const std::vector<CHAR_INFO>& GetData() const { return m_chiBuffer; }

        [[nodiscard]] uint16_t GetWidth() const { return m_uiWidth; }
        [[nodiscard]] uint16_t GetHeight() const { return m_uiHeight; }
        [[nodiscard]] bool IsClearOnPresent() const { return m_bClearOnPresent; }

        void Resize(RenderContext& kRenderContext, int iWidth, int iHeight);
        bool RequiresResize(RenderContext& kRenderContext, int iWidth, int iHeight) const;

    private:
        uint16_t m_uiWidth;
        uint16_t m_uiHeight;
        bool m_bClearOnPresent;

        std::vector<CHAR_INFO> m_chiBuffer;
    };

    using WriteSurface = ConsoleWriteSurface;
}
