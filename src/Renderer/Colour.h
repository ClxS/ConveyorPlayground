#pragma once
#include <cstdint>

namespace cpp_conv
{
	union Colour
	{
		uint32_t m_value;
		struct ArgbTag
		{
			uint8_t m_b;
			uint8_t m_g;
			uint8_t m_r;
			uint8_t m_a;
		} m_argb;
	};
}