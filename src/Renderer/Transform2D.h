#pragma once

namespace cpp_conv
{
	struct Transform2D
	{
		enum class Rotation
		{
			DegZero	= 0,
			Deg90   = 1,
			Deg180  = 2,
			Deg270  = 3,
		};

		int m_x;

		int m_y;
		
		Rotation m_rotation;
	};
}