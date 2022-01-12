#include "AppHost.h"
#include "Game.h"
#include <Windows.h>
#include <format>

std::tuple<int, int> cpp_conv::apphost::getAppDimensions()
{
    HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r);

    // current window size
    short winWidth = (short)(r.right - r.left);
    short winHeight = (short)(r.bottom - r.top);
     
    winWidth -= GetSystemMetrics(SM_CYEDGE) * 2;
    winHeight -= GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYEDGE) * 2;

    return std::make_tuple(max(0, winWidth), max(0, winHeight));
}

int main()
{
    SetConsoleOutputCP(65001);
    cpp_conv::game::run();
}
