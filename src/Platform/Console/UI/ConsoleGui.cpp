#include <cstdint>

#include "Colour.h"
#include "RenderContext.h"
#include "Gui.h"
#include "ConsoleWriteUtility.h"

void cpp_conv::ui::platform::drawText(const char* szText, cpp_conv::Colour colour, uint32_t x, uint32_t y)
{
    uint32_t designWidth, designHeight;
    std::tie(designWidth, designHeight) = getDesignDimensions();

    auto pRenderContext = getCurrentContext();
    auto& rSurface = pRenderContext->m_surface;
    float fRlativeWidth = rSurface->GetWidth() / (float)designWidth;
    float fRelativeHeight = (rSurface->GetHeight() / (float)designHeight);

    uint32_t consoleWorldX = (uint32_t)(x * fRlativeWidth);
    uint32_t consoleWorldY = (uint32_t)(y * fRelativeHeight);
    for (int i = 0; i < std::strlen(szText); ++i)
    {
        cpp_conv::renderer::setCell(*pRenderContext, szText[i], consoleWorldX + i, consoleWorldY, cpp_conv::renderer::getWin32Colour(colour));
    }
}

uint32_t getTextLineHeight()
{

}
