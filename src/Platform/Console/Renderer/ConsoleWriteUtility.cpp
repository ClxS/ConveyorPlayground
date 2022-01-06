#include "ConsoleWriteUtility.h"

WORD GetColourAttribute(int colour, bool allowBackFill)
{
	if (allowBackFill)
	{
		switch (colour % 7)
		{
		case 0: return FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_RED;
		case 1: return FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_GREEN;
		case 2: return FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
		case 3: return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE;
		case 4: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE;
		case 5: return FOREGROUND_GREEN | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_GREEN;
		case 6: return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED;
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

void cpp_conv::renderer::setCell(RenderContext& kContext, wchar_t value, int x, int y,
	int colour, bool allowBackFill)
{
	x += kContext.cameraOffsetX;
	y += kContext.cameraOffsetY;

	if (x < 0 || y < 0 || x >= kContext.m_surface.GetWidth() || y >= kContext.m_surface.GetHeight())
	{
		return;
	}

	WriteSurface& rSurface = kContext.m_surface;
	CHAR_INFO& rCell = rSurface.GetData()[x + rSurface.GetWidth() * y];

	rCell.Char.UnicodeChar = value;
	rCell.Attributes = GetColourAttribute(colour, allowBackFill);
}