#pragma once

#include <array>
#include "Position.h"

namespace cpp_conv
{
    class Entity;
    class Conveyor;

    namespace grid
    {
        using GridRow = std::array<Entity*, 64>;
        using EntityGrid = std::array<GridRow, 64>;

        Entity* SafeGetEntity(const EntityGrid& grid, Position pos);
        Position GetForwardPosition(const Conveyor& conveyor);
        Position GetRightPosition(const Conveyor& conveyor);
        Position GetBackwardsPosition(const Conveyor& conveyor);
        Position GetLeftPosition(const Conveyor& conveyor);

        bool IsConveyor(const Entity* pEntity);
    }
}