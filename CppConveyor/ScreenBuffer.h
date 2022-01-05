#pragma once

namespace cpp_conv::renderer
{
	constexpr int c_screenWidth = 256;
	constexpr int c_screenHeight = 256;

	using ScreenBuffer = wchar_t[c_screenHeight][c_screenWidth];
}