#include "Renderer.h"
#include "Entity.h"
#include "Conveyor.h"
#include "Producer.h"

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

void cpp_conv::renderer::setPixel(HANDLE hConsole, cpp_conv::renderer::ScreenBuffer screenBuffer, wchar_t value, int x, int y,
    int colour, bool allowBackFill)
{
    if (screenBuffer[y][x] == value)
    {
        return;
    }

    screenBuffer[y][x] = value;

    COORD pos = { (SHORT)x, (SHORT)y };
    DWORD dwBytesWritten = 0;

    SetConsoleActiveScreenBuffer(hConsole);

    WORD attribute = GetColourAttribute(colour, allowBackFill);
    WriteConsoleOutputAttribute(hConsole, &attribute, 1, pos, &dwBytesWritten);
    WriteConsoleOutputCharacterW(hConsole, &value, 1, pos, &dwBytesWritten);
}

void cpp_conv::renderer::init(HANDLE& hConsole)
{
    hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof cfi;
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 18;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_HEAVY;

    wcscpy_s<32>(cfi.FaceName, L"Lucida Console");
    SetCurrentConsoleFontEx(hConsole, FALSE, &cfi);
}

void cpp_conv::renderer::render(HANDLE hConsole, cpp_conv::renderer::ScreenBuffer screenBuffer, cpp_conv::grid::EntityGrid& grid)
{
    for (int y = 0; y < grid.size(); y++)
    {
        for (int x = 0; x < grid[y].size(); x++)
        {
            auto cell = grid[y][x];
            if (cell == nullptr)
            {
                //cpp_conv::renderer::setPixel(hConsole, screenBuffer, L' ', x * cpp_conv::renderer::c_gridScale, y * cpp_conv::renderer::c_gridScale, 0);
            }
            else
            {
                cell->Draw(hConsole, screenBuffer, grid, x, y);
            }
        }
    }
}