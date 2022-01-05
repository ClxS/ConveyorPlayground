#include "WriteSurface.h"

cpp_conv::renderer::WriteSurface::WriteSurface(uint16_t uiWidth, uint16_t uiHeight, bool bClearOnPresent)
	: m_uiWidth(uiWidth)
	, m_uiHeight(uiHeight)
	, m_bClearOnPresent(bClearOnPresent)
{
}

void cpp_conv::renderer::WriteSurface::Initialize()
{
	m_chiBuffer.resize(m_uiWidth * m_uiHeight);
}

void cpp_conv::renderer::WriteSurface::Clear()
{
	CHAR_INFO emptyChar;
	emptyChar.Char.UnicodeChar = 0;
	emptyChar.Attributes = 0;

	std::fill(m_chiBuffer.begin(), m_chiBuffer.end(), emptyChar);
}
