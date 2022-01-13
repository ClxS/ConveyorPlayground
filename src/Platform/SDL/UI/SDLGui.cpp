#include <cstdint>

#include "Gui.h"
#include "RenderContext.h"
#include "AppHost.h"

void cpp_conv::ui::platform::drawText(const std::string& szText, cpp_conv::Colour colour, uint32_t x, uint32_t y)
{
}

void cpp_conv::ui::platform::drawText(const std::wstring& szText, cpp_conv::Colour colour, uint32_t x, uint32_t y)
{
}

void cpp_conv::ui::platform::drawWrappedText(const std::string& szText, cpp_conv::Colour colour, uint32_t x, uint32_t y, uint32_t& linesRequired)
{
}

uint32_t cpp_conv::ui::platform::getTextLineHeight()
{
    return 10;
}
