#include "AppHost.h"
#include "Game.h"
#include <Windows.h>

std::tuple<int, int> cpp_conv::apphost::getAppDimensions()
{
    HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r); //stores the console's current dimensions

    // current window size
    short winWidth = (short)(r.right - r.left + 1);
    short winHeight = (short)(r.bottom - r.top + 1);

    return std::make_tuple(winWidth, winHeight);
}

int main()
{
    cpp_conv::game::run();
}