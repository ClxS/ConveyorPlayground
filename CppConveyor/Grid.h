#pragma once

#include <array>
#include "Entity.h"
#include "Conveyor.h"

namespace cpp_conv
{
    namespace grid
    {
        using GridRow = std::array<Entity*, 64>;
        using EntityGrid = std::array<GridRow, 64>;

        Entity* SafeGetEntity(const EntityGrid & grid, Position pos)
        {
            if (pos.m_x < 0 || pos.m_y < 0 || pos.m_y >= grid.size() || pos.m_x >= grid[pos.m_y].size())
            {
                return nullptr;
            }

            return grid[pos.m_y][pos.m_x];
        }

        Position GetForwardPosition(const Conveyor & conveyor)
        {
            Position facingDirection = { 0, 0 };

            switch (conveyor.m_direction)
            {
            case Direction::Left: facingDirection = { -1, 0 }; break;
            case Direction::Up: facingDirection = { 0, 1 }; break;
            case Direction::Right: facingDirection = { 1, 0 }; break;
            case Direction::Down: facingDirection = { 0, -1 }; break;
            }

            return conveyor.m_position + facingDirection;
        }

        Position GetRightPosition(const Conveyor & conveyor)
        {
            Position facingDirection = { 0, 0 };

            switch (conveyor.m_direction)
            {
            case Direction::Left: facingDirection = { 0, 1 }; break;
            case Direction::Up: facingDirection = { 1, 0 }; break;
            case Direction::Right: facingDirection = { 0, -1 }; break;
            case Direction::Down: facingDirection = { -1, 0 }; break;
            }

            return conveyor.m_position + facingDirection;
        }

        Position GetBackwardsPosition(const Conveyor & conveyor)
        {
            Position pos = GetForwardPosition(conveyor) - conveyor.m_position;
            return conveyor.m_position - pos;
        }

        Position GetLeftPosition(const Conveyor & conveyor)
        {
            Position pos = GetRightPosition(conveyor) - conveyor.m_position;
            return conveyor.m_position - pos;
        }

        bool IsConveyor(const Entity* pEntity)
        {
            return pEntity != nullptr && pEntity->m_eEntityKind == EntityKind::Conveyor;
        }
    }
}