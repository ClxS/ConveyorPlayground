#pragma once

#include "Enums.h"

namespace cpp_conv::direction
{
    inline Direction Rotate90DegreeClockwise(Direction direction)
    {
        switch (direction)
        {
        case Direction::Down: return Direction::Right;
        case Direction::Right: return Direction::Up;
        case Direction::Up: return Direction::Left;
        case Direction::Left: return Direction::Down;
        }
        return Direction::Up;
    }

    inline Direction Rotate90DegreeCounterClockwise(Direction direction)
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
}
