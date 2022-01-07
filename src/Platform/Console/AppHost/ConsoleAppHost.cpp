#include "AppHost.h"
#include "Game.h"
#include <Windows.h>

static int g_appWidth = 0;
static int g_appHeight = 0;

std::tuple<int, int> cpp_conv::apphost::getAppDimensions()
{
    if (g_appWidth == 0 || g_appHeight == 0)
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        int columns, rows;

        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        g_appWidth = columns;
        g_appHeight = rows;
    }

    return std::make_tuple(g_appWidth, g_appHeight);
}

void cpp_conv::apphost::setAppDimensions(int width, int height)
{
    g_appWidth = width;
    g_appHeight = height;
}

int main()
{
    cpp_conv::game::run();
}