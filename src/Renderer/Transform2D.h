#pragma once
#include "Enums.h"

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

		Transform2D()
			: m_x(0)
			, m_y(0)
			, m_rotation(Rotation::DegZero)
		{
		}

		Transform2D(int x, int y)
			: m_x(x)
			, m_y(y)
			, m_rotation(Rotation::DegZero)
		{
		}

		Transform2D(int x, int y, Rotation rotation)
			: m_x(x)
			, m_y(y)
			, m_rotation(rotation)
		{
		}

		int m_x;

		int m_y;
		
		Rotation m_rotation;
	};

	inline Transform2D::Rotation rotationFromDirection(Direction direction)
	{
		// This engine uses right-based assets. Up/Down are inverted as we use a top major positioning system
		switch (direction)
		{
		default:
		case Direction::Right:
			return cpp_conv::Transform2D::Rotation::DegZero;
		case Direction::Up:
			return cpp_conv::Transform2D::Rotation::Deg90;
		case Direction::Left:
			return cpp_conv::Transform2D::Rotation::Deg180;
		case Direction::Down:
			return cpp_conv::Transform2D::Rotation::Deg270;
		}
	}
}