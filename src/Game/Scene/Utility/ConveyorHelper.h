#pragma once

#include "Direction.h"

#include <AtlasScene/ECS/Entity.h>
#include <Eigen/Core>

#include "ConveyorComponent.h"
#include "SequenceComponent.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

namespace cpp_conv
{
    class EntityLookupGrid;
}

namespace atlas::scene
{
    class EcsManager;
}

namespace cpp_conv::conveyor_helper
{
    struct InsertInfo
    {
        ItemId m_Item{};
        std::optional<Eigen::Vector2f> m_OriginPosition{};
    };

    atlas::scene::EntityId findNextTailConveyor(
        const atlas::scene::EcsManager& ecs,
        const EntityLookupGrid& grid,
        Eigen::Vector3i position,
        Direction direction,
        RelativeDirection& outDirection);

    inline bool hasItemInSlot(
        const components::SequenceComponent& sequence,
        const uint8_t sequenceIndex,
        const int channel,
        const int slot);

    inline bool hasItemInSlot(
        const atlas::scene::EcsManager& ecs,
        const components::ConveyorComponent& conveyor,
        int lane,
        int slot);

    inline void placeItemInSlot(
        atlas::scene::EcsManager& ecs,
        components::SequenceComponent& sequence,
        uint8_t sequenceIndex,
        int targetChannel,
        int targetSlot,
        const InsertInfo& info);

    inline void placeItemInSlot(
        atlas::scene::EcsManager& ecs,
        components::ConveyorComponent& conveyor,
        int targetChannel,
        int targetSlot,
        const InsertInfo& info,
        bool bShouldSetDirectly = false);
}
