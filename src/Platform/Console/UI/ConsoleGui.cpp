#include <cstdint>

#include "Colour.h"
#include "RenderContext.h"
#include "Gui.h"
#include "ConsoleWriteUtility.h"

void cpp_conv::ui::platform::drawText(const std::string& szText, cpp_conv::Colour colour, uint32_t x, uint32_t y)
{
    uint32_t designWidth, designHeight;
    std::tie(designWidth, designHeight) = getDesignDimensions();

    auto pRenderContext = getCurrentContext();
    auto& rSurface = pRenderContext->m_surface;
    float fRlativeWidth = rSurface->GetWidth() / (float)designWidth;
    float fRelativeHeight = (rSurface->GetHeight() / (float)designHeight);

    uint32_t consoleWorldX = (uint32_t)(x * fRlativeWidth);
    uint32_t consoleWorldY = (uint32_t)(y * fRelativeHeight);
    for (int i = 0; i < szText.length(); ++i)
    {
        cpp_conv::renderer::setCell(*pRenderContext, szText[i], consoleWorldX + i, consoleWorldY, cpp_conv::renderer::getWin32Colour(colour), true);
    }
}

void cpp_conv::ui::platform::drawWrappedText(const std::string& szText, cpp_conv::Colour colour, uint32_t x, uint32_t y, uint32_t& linesRequired)
{
    uint32_t designWidth, designHeight;
    std::tie(designWidth, designHeight) = getDesignDimensions();

    auto pRenderContext = getCurrentContext();
    auto& rSurface = pRenderContext->m_surface;
    float fRlativeWidth = rSurface->GetWidth() / (float)designWidth;
    float fRelativeHeight = (rSurface->GetHeight() / (float)designHeight);

    uint32_t consoleWorldX = (uint32_t)(x * fRlativeWidth);
    uint32_t consoleWorldY = (uint32_t)(y * fRelativeHeight);
    uint32_t col = 0;
    uint32_t row = 0;    
    for (int i = 0; i < szText.length(); ++i)
    {
        cpp_conv::renderer::setCell(*pRenderContext, szText[i], consoleWorldX + col, consoleWorldY + row, cpp_conv::renderer::getWin32Colour(colour), true);
        col++;

        if (consoleWorldX + col >= rSurface->GetWidth())
        {
            col = 0;
            row++;
        }
    }

    linesRequired = row + 1;
}

uint32_t cpp_conv::ui::platform::getTextLineHeight()
{
    cpp_conv::RenderContext* pContext = getCurrentContext();
    return 18 * pContext->m_fZoom;
}
