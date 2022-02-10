#include "EntityGrid.h"

#include "Entity.h"

Vector3 cpp_conv::grid::getForwardPosition(const Vector3& position, Direction direction)
{
    Vector3 facingDirection;

    switch (direction)
    {
    case Direction::Left: facingDirection = { -1, 0, 0 }; break;
    case Direction::Up: facingDirection = { 0, 1, 0 }; break;
    case Direction::Right: facingDirection = { 1, 0, 0 }; break;
    case Direction::Down: facingDirection = { 0, -1, 0 }; break;
    }

    return (Vector3)(position + facingDirection);
}

Vector3 cpp_conv::grid::getForwardPosition(const cpp_conv::Entity& rEntity)
{
    return getForwardPosition(rEntity.m_position, rEntity.GetDirection());
}

Vector3 cpp_conv::grid::getRightPosition(const cpp_conv::Entity& rEntity)
{
    Vector3 facingDirection = { 0, 0, 0 };

    switch (rEntity.GetDirection())
    {
    case Direction::Left: facingDirection = { 0, 1, 0 }; break;
    case Direction::Up: facingDirection = { 1, 0, 0 }; break;
    case Direction::Right: facingDirection = { 0, -1, 0 }; break;
    case Direction::Down: facingDirection = { -1, 0, 0 }; break;
    }

    return rEntity.m_position + facingDirection;
}

Vector3 cpp_conv::grid::getBackwardsPosition(const cpp_conv::Entity& rEntity)
{
    const Vector3 pos = getForwardPosition(rEntity) - rEntity.m_position;
    return rEntity.m_position - pos;
}

Vector3 cpp_conv::grid::getLeftPosition(const cpp_conv::Entity& rEntity)
{
    const Vector3 pos = getRightPosition(rEntity) - rEntity.m_position;
    return rEntity.m_position - pos;
}
