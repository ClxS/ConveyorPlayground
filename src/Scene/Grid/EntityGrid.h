#pragma once

#include <array>
#include "Vector3.h"
#include "Enums.h"
#include "Entity.h"

namespace cpp_conv
{
    class Entity;
    class Conveyor;

    namespace grid
    {
        Vector3 GetForwardPosition(const Vector3& position, Direction direction);
        Vector3 GetForwardPosition(const Entity& rEntity);
        Vector3 GetRightPosition(const Entity& rEntity);
        Vector3 GetBackwardsPosition(const Entity& rEntity);
        Vector3 GetLeftPosition(const Entity& rEntity);
    }
}
