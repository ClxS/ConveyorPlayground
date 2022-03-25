#pragma once

#include "Direction.h"

#include <AtlasScene/ECS/Entity.h>
#include <Eigen/Core>

namespace cpp_conv
{
    class EntityLookupGrid;
}

namespace atlas::scene
{
    class EcsManager;
}

namespace cpp_conv::conveyor_tracing_utility
{
    atlas::scene::EntityId findNextTailConveyor(
        const atlas::scene::EcsManager& ecs,
        const EntityLookupGrid& grid,
        Eigen::Vector3i position,
        Direction direction,
        RelativeDirection& outDirection);
}
