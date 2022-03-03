#pragma once

#include "Entity.h"
#include "Enums.h"
#include "Vector3.h"

namespace cpp_conv
{
    class Entity;
    class Conveyor;

    namespace grid
    {
        Vector3 getForwardPosition(const Vector3& position, Direction direction);
        Vector3 getBackwardsPosition(const Vector3& position, Direction direction);
        Vector3 getForwardPosition(const Entity& rEntity);
        Vector3 getRightPosition(const Entity& rEntity);
        Vector3 getBackwardsPosition(const Entity& rEntity);

        Vector3 getLeftPosition(const Entity& rEntity);
    }
}
