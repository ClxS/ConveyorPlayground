#include "Grid.h"

#include "Entity.h"
#include "Conveyor.h"

cpp_conv::Entity* cpp_conv::grid::SafeGetEntity(const EntityGrid& grid, Position pos)
{
    if (pos.m_x < 0 || pos.m_y < 0 || pos.m_y >= grid.size() || pos.m_x >= grid[pos.m_y].size())
    {
        return nullptr;
    }

    return grid[pos.m_y][pos.m_x];
}

Position cpp_conv::grid::GetForwardPosition(const Position& position, Direction direction)
{
    Position facingDirection = { 0, 0 };

    switch (direction)
    {
    case Direction::Left: facingDirection = { -1, 0 }; break;
    case Direction::Up: facingDirection = { 0, 1 }; break;
    case Direction::Right: facingDirection = { 1, 0 }; break;
    case Direction::Down: facingDirection = { 0, -1 }; break;
    }

    return position + facingDirection;
}

Position cpp_conv::grid::GetForwardPosition(const cpp_conv::Entity& entity, Direction direction)
{
    return GetForwardPosition(entity.m_position, direction);
}

Position cpp_conv::grid::GetForwardPosition(const cpp_conv::Conveyor& conveyor)
{    
    return GetForwardPosition(conveyor, conveyor.m_direction);
}

Position cpp_conv::grid::GetRightPosition(const cpp_conv::Conveyor& conveyor)
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

Position cpp_conv::grid::GetBackwardsPosition(const cpp_conv::Conveyor& conveyor)
{
    Position pos = GetForwardPosition(conveyor) - conveyor.m_position;
    return conveyor.m_position - pos;
}

Position cpp_conv::grid::GetLeftPosition(const cpp_conv::Conveyor& conveyor)
{
    Position pos = GetRightPosition(conveyor) - conveyor.m_position;
    return conveyor.m_position - pos;
}

bool cpp_conv::grid::IsConveyor(const cpp_conv::Entity* pEntity)
{
    return pEntity != nullptr && pEntity->m_eEntityKind == EntityKind::Conveyor;
}
