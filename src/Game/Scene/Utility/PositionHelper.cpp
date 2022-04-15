#include "PositionHelper.h"

Eigen::Vector3i cpp_conv::position_helper::getForwardPosition(const Eigen::Vector3i& position, const Direction direction)
{
    Eigen::Vector3i facingDirection;
    switch (direction)
    {
    case Direction::Left: facingDirection = {-1, 0, 0};
        break;
    case Direction::Up: facingDirection = {0, 0, 1};
        break;
    case Direction::Right: facingDirection = {1, 0, 0};
        break;
    case Direction::Down: facingDirection = {0, 0, -1};
        break;
    }
    return position + facingDirection;
}

Eigen::Vector3i cpp_conv::position_helper::getBackwardsPosition(const Eigen::Vector3i& position, Direction direction)
{
    const Eigen::Vector3i pos = getForwardPosition(position, direction) - position;
    return position - pos;
}

Eigen::Vector3i cpp_conv::position_helper::getRightPosition(const Eigen::Vector3i& position, Direction direction)
{
    Eigen::Vector3i facingDirection = {0, 0, 0};
    switch (direction)
    {
    case Direction::Left: facingDirection = {0, 0, 1};
        break;
    case Direction::Up: facingDirection = {1, 0, 0};
        break;
    case Direction::Right: facingDirection = {0, 0, -1};
        break;
    case Direction::Down: facingDirection = {-1, 0, 0};
        break;
    }
    return position + facingDirection;
}

Eigen::Vector3i cpp_conv::position_helper::getLeftPosition(const Eigen::Vector3i& position, Direction direction)
{
    const Eigen::Vector3i pos = getRightPosition(position, direction) - position;
    return position - pos;
}
