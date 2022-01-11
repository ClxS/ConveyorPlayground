#pragma once

#include <array>
#include "Position.h"
#include "Enums.h"
#include "Entity.h"

namespace cpp_conv
{
    class Entity;
    class Conveyor;

    namespace grid
    {
        Position GetForwardPosition(const Position& position, Direction direction);
        Position GetForwardPosition(const Entity& rEntity);
        Position GetRightPosition(const Entity& rEntity);
        Position GetBackwardsPosition(const Entity& rEntity);
        Position GetLeftPosition(const Entity& rEntity);
    }
}
