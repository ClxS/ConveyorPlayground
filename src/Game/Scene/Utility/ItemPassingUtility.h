#pragma once

#include <AtlasScene/ECS/Entity.h>
#include "DataId.h"
#include "Eigen/Core"

namespace cpp_conv
{
    class EntityLookupGrid;
}

namespace atlas::scene
{
    class EcsManager;
}

namespace cpp_conv::item_passing_utility
{
    bool entitySupportsInsertion(
        const atlas::scene::EcsManager& ecs,
        atlas::scene::EntityId targetEntity);

    bool tryInsertItem(
        atlas::scene::EcsManager& ecs,
        const EntityLookupGrid& grid,
        atlas::scene::EntityId sourceEntity,
        atlas::scene::EntityId targetEntity,
        ItemId itemId,
        std::optional<int> sourceChannel,
        std::optional<Eigen::Vector2f> startPosition);
}
