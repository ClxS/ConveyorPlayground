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
        using GridRow = std::array<Entity*, 64>;
        using EntityGrid = std::array<GridRow, 64>;

        Entity* SafeGetEntity(const EntityGrid& grid, Position pos);
        Position GetForwardPosition(const Position& position, Direction direction);
        Position GetForwardPosition(const Entity& entity, Direction direction);
        Position GetForwardPosition(const Conveyor& conveyor);
        Position GetRightPosition(const Conveyor& conveyor);
        Position GetBackwardsPosition(const Conveyor& conveyor);
        Position GetLeftPosition(const Conveyor& conveyor);

        bool IsConveyor(const Entity* pEntity);

        template<typename TEntity>
        TEntity* SafeGetEntity(const EntityGrid& grid, Position pos, EntityKind expectedKind)
        {
            Entity* pEntity = SafeGetEntity(grid, pos);
            if (pEntity == nullptr || pEntity->m_eEntityKind != expectedKind)
            {
                return nullptr;
            }

            return reinterpret_cast<TEntity*>(pEntity);
        }
    }
}