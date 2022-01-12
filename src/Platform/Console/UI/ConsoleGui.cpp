#include <cstdint>

#include "Colour.h"
#include "RenderContext.h"
#include "Gui.h"
#include "ConsoleWriteUtility.h"
#include "AppHost.h"

void cpp_conv::ui::platform::drawText(const std::string& szText, cpp_conv::Colour colour, uint32_t x, uint32_t y)
{
    uint32_t designWidth, designHeight;
    std::tie(designWidth, designHeight) = getDesignDimensions();

    auto pRenderContext = getCurrentContext();
    auto& rSurface = pRenderContext->m_surface;
    if (rSurface->GetHeight() == 0)
    {
        return;
    }

    cpp_conv::RenderContext* pContext = getCurrentContext();

    uint32_t appWidth, appHeight;
    std::tie(appWidth, appHeight) = cpp_conv::apphost::getAppDimensions();

    uint32_t consoleWorldX = (uint32_t)((((double)x / designWidth) * 1.05f) * rSurface->GetWidth());
    uint32_t consoleWorldY = (uint32_t)((((double)y / designHeight) * 1.05f) * rSurface->GetHeight());
    for (int i = 0; i < szText.length(); ++i)
    {
        cpp_conv::renderer::setCell(*pRenderContext, szText[i], consoleWorldX + i, consoleWorldY, cpp_conv::renderer::getWin32Colour(colour), true);
    }
}

void cpp_conv::ui::platform::drawText(const std::wstring& szText, cpp_conv::Colour colour, uint32_t x, uint32_t y)
{
    uint32_t designWidth, designHeight;
    std::tie(designWidth, designHeight) = getDesignDimensions();

    auto pRenderContext = getCurrentContext();
    auto& rSurface = pRenderContext->m_surface;
    if (rSurface->GetHeight() == 0)
    {
        return;
    }

    cpp_conv::RenderContext* pContext = getCurrentContext();

    uint32_t appWidth, appHeight;
    std::tie(appWidth, appHeight) = cpp_conv::apphost::getAppDimensions();

    uint32_t consoleWorldX = (uint32_t)((((double)x / designWidth) * 1.05f) * rSurface->GetWidth());
    uint32_t consoleWorldY = (uint32_t)((((double)y / designHeight) * 1.05f) * rSurface->GetHeight());
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
    if (rSurface->GetHeight() == 0)
    {
        return;
    }

    cpp_conv::RenderContext* pContext = getCurrentContext();

    uint32_t appWidth, appHeight;
    std::tie(appWidth, appHeight) = cpp_conv::apphost::getAppDimensions();

    uint32_t consoleWorldX = (uint32_t)((((double)x / designWidth) * 1.05f) * rSurface->GetWidth());
    uint32_t consoleWorldY = (uint32_t)((((double)y / designHeight) * 1.05f) * rSurface->GetHeight());
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
    uint32_t designWidth, designHeight;
    std::tie(designWidth, designHeight) = getDesignDimensions();

    auto pRenderContext = getCurrentContext();
    auto& rSurface = pRenderContext->m_surface;
    if (rSurface->GetHeight() == 0)
    {
        return 0;
    }

    return (designHeight) / rSurface->GetHeight();
}
