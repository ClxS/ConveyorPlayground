#pragma once

#include "Entity.h"
#include "FactoryDefinition.h"

namespace cpp_conv
{
    class LaunchPad final : public Entity
    {
    public:
        LaunchPad(const Eigen::Vector3i position, const Direction direction)
        : Entity(position, {10, 4, 10}, EntityKind::LaunchPad, direction)
        {
        }
    };
}
