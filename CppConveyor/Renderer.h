#pragma once

#include <Windows.h>
#include "Grid.h"

namespace cpp_conv
{
	namespace renderer
	{
		constexpr int c_screenWidth = 256;
		constexpr int c_screenHeight = 256;
		constexpr int c_gridScale = 3;

		using ScreenBuffer = wchar_t[c_screenHeight][c_screenWidth];

		void init(HANDLE& hConsole);
        void render(HANDLE hConsole, ScreenBuffer screenBuffer, cpp_conv::grid::EntityGrid& grid);

		void setPixel(HANDLE hConsole, cpp_conv::renderer::ScreenBuffer screenBuffer, wchar_t value, int x, int y, int colour, bool allowBackFill = false);
	}
}