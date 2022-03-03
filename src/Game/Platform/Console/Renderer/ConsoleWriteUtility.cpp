#include "ConsoleWriteUtility.h"
#include "Colour.h"

WORD cpp_conv::renderer::getWin32Colour(cpp_conv::Colour colour)
{
    WORD value = {};
    if (colour.m_argb.m_a == 0xFF)
    {
        if (colour.m_argb.m_r)
        {
            value |= FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_RED;
        }

        if (colour.m_argb.m_g)
        {
            value |= FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_GREEN;
        }

        if (colour.m_argb.m_b)
        {
            value |= FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
        }
    }
    else
    {
        if (colour.m_argb.m_r)
        {
            value |= FOREGROUND_RED | FOREGROUND_INTENSITY;
        }

        if (colour.m_argb.m_g)
        {
            value |= FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        }

        if (colour.m_argb.m_b)
        {
            value |= FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        }
    }

    if (value == 0)
    {
        value = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    }

    return value;
}

void cpp_conv::renderer::setCell(RenderContext& kContext, wchar_t value, int x, int y, WORD colour, bool bSuppressPan)
{
    if (!bSuppressPan)
    {
        x -= kContext.m_cameraQuad.GetLeft();
        y -= kContext.m_cameraQuad.GetTop();
    }

    if (kContext.m_surface == nullptr || x < 0 || y < 0 || x >= kContext.m_surface->GetWidth() || y >= kContext.m_surface->GetHeight())
    {
        return;
    }

    WriteSurface& rSurface = *kContext.m_surface;
    CHAR_INFO& rCell = rSurface.GetData()[x + rSurface.GetWidth() * y];

    rCell.Char.UnicodeChar = value;
    rCell.Attributes = colour;
}

std::tuple<uint32_t, uint32_t> cpp_conv::renderer::getConsoleSpaceDimensions(uint32_t uiWidth, uint32_t uiHeight, float scale)
{
    if (uiWidth == 0 || uiHeight == 9)
    {
        return std::make_tuple(0, 0);
    }

    return std::make_tuple((uint32_t)(uiWidth / (19.0f * scale) - 0), (uint32_t)(uiHeight / (19.0f * scale) - 0));
}