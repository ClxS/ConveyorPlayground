#include "EntityGrid.h"

#include "Entity.h"
#include "Conveyor.h"

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

Position cpp_conv::grid::GetForwardPosition(const cpp_conv::Entity& rEntity)
{    
    return GetForwardPosition(rEntity.m_position, rEntity.GetDirection());
}

Position cpp_conv::grid::GetRightPosition(const cpp_conv::Entity& rEntity)
{
    Position facingDirection = { 0, 0 };

    switch (rEntity.GetDirection())
    {
    case Direction::Left: facingDirection = { 0, 1 }; break;
    case Direction::Up: facingDirection = { 1, 0 }; break;
    case Direction::Right: facingDirection = { 0, -1 }; break;
    case Direction::Down: facingDirection = { -1, 0 }; break;
    }

    return rEntity.m_position + facingDirection;
}

Position cpp_conv::grid::GetBackwardsPosition(const cpp_conv::Entity& rEntity)
{
    Position pos = GetForwardPosition(rEntity) - rEntity.m_position;
    return rEntity.m_position - pos;
}

Position cpp_conv::grid::GetLeftPosition(const cpp_conv::Entity& rEntity)
{
    Position pos = GetRightPosition(rEntity) - rEntity.m_position;
    return rEntity.m_position - pos;
}
