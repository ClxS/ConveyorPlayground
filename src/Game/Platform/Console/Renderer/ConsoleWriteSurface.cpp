#include "ConsoleWriteSurface.h"
#include "ConsoleWriteUtility.h"

cpp_conv::renderer::ConsoleWriteSurface::ConsoleWriteSurface(cpp_conv::RenderContext& kRenderContext, uint16_t uiWidth, uint16_t uiHeight, bool bClearOnPresent)
    : m_bClearOnPresent(bClearOnPresent)
{
    std::tie(m_uiWidth, m_uiHeight) = getConsoleSpaceDimensions(uiWidth, uiHeight, kRenderContext.m_fZoom);
}

void cpp_conv::renderer::ConsoleWriteSurface::Initialize()
{
    m_chiBuffer.resize(m_uiWidth * m_uiHeight);
    Clear();
}

void cpp_conv::renderer::ConsoleWriteSurface::Clear()
{
    CHAR_INFO emptyChar;
    emptyChar.Char.UnicodeChar = 0;
    emptyChar.Attributes = 0;

    std::fill(m_chiBuffer.begin(), m_chiBuffer.end(), emptyChar);
}

void cpp_conv::renderer::ConsoleWriteSurface::Resize(cpp_conv::RenderContext& kRenderContext, int iWidth, int iHeight)
{
    std::tie(m_uiWidth, m_uiHeight) = getConsoleSpaceDimensions(iWidth, iHeight, kRenderContext.m_fZoom);
    Initialize();
}

bool cpp_conv::renderer::ConsoleWriteSurface::RequiresResize(cpp_conv::RenderContext& kRenderContext, int iWidth, int iHeight) const
{
    uint32_t newWidth, newHeight;
    std::tie(newWidth, newHeight) = getConsoleSpaceDimensions(iWidth, iHeight, kRenderContext.m_fZoom);

    return m_uiWidth != newWidth || m_uiHeight != newHeight;
}
