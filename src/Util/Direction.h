#pragma once

#include "Enums.h"

namespace cpp_conv::direction
{
    inline Direction Rotate90DegreeClockwise(Direction direction)
    {
        switch (direction)
        {
        case Direction::Up: return Direction::Right;
        case Direction::Right: return Direction::Down;
        case Direction::Down: return Direction::Left;
        case Direction::Left: return Direction::Up;
        }
        return Direction::Up;
    }

    inline Direction Rotate90DegreeCounterClockwise(Direction direction)
    {
        switch (direction)
        {
        case Direction::Up: return Direction::Left;
        case Direction::Right: return Direction::Up;
        case Direction::Down: return Direction::Right;
        case Direction::Left: return Direction::Down;
        }
        return Direction::Up;
    }

    inline Vector3 RotateVector(Direction direction, Vector3 location, Vector3 size)
    {
        constexpr Vector3 c_offset = { 1, 1, 0 };
        size -= c_offset;
        switch (direction)
        {
        case Direction::Up: return { size.m_y - location.m_y, location.m_x, location.m_depth };
        case Direction::Left: return { size.m_x - location.m_x, size.m_y - location.m_y, location.m_depth };
        case Direction::Down: return { size.m_y - location.m_y, size.m_x - location.m_x, location.m_depth };
        }

        return location;
    }
}
