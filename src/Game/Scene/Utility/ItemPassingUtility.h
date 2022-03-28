#pragma once

#include <AtlasScene/ECS/Entity.h>
#include "DataId.h"
#include "Eigen/Core"

namespace cpp_conv
{
    class EntityLookupGrid;
}

namespace atlas
{
    namespace scene
    {
        class EcsManager;
    }
}

namespace cpp_conv::item_passing_utility
{
    bool tryInsertItem(
        atlas::scene::EcsManager& ecs,
        const EntityLookupGrid& grid,
        atlas::scene::EntityId sourceEntity,
        atlas::scene::EntityId targetEntity,
        ItemId itemId,
        int sourceChannel,
        Eigen::Vector2f startPosition);
}
