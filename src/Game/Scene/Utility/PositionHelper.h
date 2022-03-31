#pragma once

#include <Eigen/Core>

#include "Direction.h"

namespace cpp_conv::position_helper
{
    Eigen::Vector3i getForwardPosition(const Eigen::Vector3i& position, Direction direction);
    Eigen::Vector3i getBackwardsPosition(const Eigen::Vector3i& position, Direction direction);
    Eigen::Vector3i getRightPosition(const Eigen::Vector3i& position, Direction direction);
    Eigen::Vector3i getLeftPosition(const Eigen::Vector3i& position, Direction direction);
}
