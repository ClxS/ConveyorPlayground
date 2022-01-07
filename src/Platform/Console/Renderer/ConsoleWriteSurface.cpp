#include "ConsoleWriteSurface.h"

cpp_conv::renderer::ConsoleWriteSurface::ConsoleWriteSurface(uint16_t uiWidth, uint16_t uiHeight, bool bClearOnPresent)
    : m_uiWidth(uiWidth)
    , m_uiHeight(uiHeight)
    , m_bClearOnPresent(bClearOnPresent)
{
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

void cpp_conv::renderer::ConsoleWriteSurface::Resize(int iWidth, int iHeight)
{
    m_uiWidth = iWidth;
    m_uiHeight = iHeight;
    Initialize();
}
