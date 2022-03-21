#pragma once

#include "Entity.h"
#include "Enums.h"
#include "Vector3.h"
#include "Eigen/Core"

namespace cpp_conv
{
    class Entity;
    class Conveyor;

    namespace grid
    {
        Eigen::Vector3i getForwardPosition(const Eigen::Vector3i& position, Direction direction);
        Eigen::Vector3i getBackwardsPosition(const Eigen::Vector3i& position, Direction direction);
        Eigen::Vector3i getRightPosition(const Eigen::Vector3i& position, Direction direction);
        Eigen::Vector3i getLeftPosition(const Eigen::Vector3i& position, Direction direction);


        Vector3 getForwardPosition(const Entity& rEntity);
        Vector3 getForwardPosition(const Vector3& position, Direction direction);
        Vector3 getBackwardsPosition(const Vector3& position, Direction direction);
        Vector3 getBackwardsPosition(const Entity& rEntity);
        Vector3 getRightPosition(const Entity& rEntity);
        Vector3 getLeftPosition(const Entity& rEntity);
    }
}
