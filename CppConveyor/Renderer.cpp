#include "Renderer.h"
#include "RenderContext.h"
#include "SwapChain.h"
#include "Entity.h"

WORD GetColourAttribute(int colour, bool allowBackFill)
{
    if (allowBackFill)
    {
        switch (colour % 6)
        {
        case 0: return FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_RED;
        case 1: return FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_GREEN;
        case 2: return FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
        case 3: return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE;
        case 4: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE;
        case 5: return FOREGROUND_GREEN | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_GREEN;
        }
    }
    else
    {
        switch (colour % 6)
        {
        case 0: return FOREGROUND_RED | FOREGROUND_INTENSITY;
        case 1: return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case 2: return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        case 3: return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        case 4: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        case 5: return FOREGROUND_GREEN | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        }
    }

    return FOREGROUND_RED | FOREGROUND_INTENSITY;
}

void cpp_conv::renderer::setPixel(RenderContext& kContext, wchar_t value, int x, int y,
    int colour, bool allowBackFill)
{
    WriteSurface& rSurface = kContext.m_surface;
    CHAR_INFO& rCell = rSurface.GetData()[x + rSurface.GetWidth() * y];

    rCell.Char.UnicodeChar = value;
    rCell.Attributes = GetColourAttribute(colour, allowBackFill);
}

void cpp_conv::renderer::init(cpp_conv::renderer::SwapChain& rSwapChain)
{
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof cfi;
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 18;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_HEAVY;

    wcscpy_s<32>(cfi.FaceName, L"Lucida Console");
    SurfaceInitArgs kArgs = { cfi };

    rSwapChain.Initialize(kArgs);
}

void cpp_conv::renderer::render(RenderContext& kContext)
{
    for (int y = 0; y < kContext.m_grid.size(); y++)
    {
        for (int x = 0; x < kContext.m_grid[y].size(); x++)
        {
            Entity* cell = kContext.m_grid[y][x];
            if (cell == nullptr)
            {
                //cpp_conv::renderer::setPixel(hConsole, screenBuffer, L' ', x * cpp_conv::renderer::c_gridScale, y * cpp_conv::renderer::c_gridScale, 0);
            }
            else
            {
                cell->Draw(kContext);
            }
        }
    }
}