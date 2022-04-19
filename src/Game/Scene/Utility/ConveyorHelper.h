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

    struct ItemInformation
    {
        ItemId m_Item;
        Eigen::Vector2f m_PreviousVisualLocation;
        bool m_bIsAnimated;
    };

    atlas::scene::EntityId findNextTailConveyor(
        const atlas::scene::EcsManager& ecs,
        const EntityLookupGrid& grid,
        Eigen::Vector3i position,
        Direction direction,
        RelativeDirection& outDirection);

    bool hasItemInSlot(
        const components::SequenceComponent& sequence,
        uint8_t sequenceIndex,
        int channel,
        int slot);

    inline bool hasRealizedItemInSlot(
        const components::SequenceComponent& sequence,
        const uint8_t sequenceIndex,
        const int channel,
        const int slot)
    {
        const uint64_t slotOffset = sequence.m_Length * 2 - sequenceIndex * 2 - slot - 1;
        return (sequence.m_RealizedStates[channel].m_Lanes & (1ULL << slotOffset)) != 0;
    }

    bool hasItemInSlot(
        const atlas::scene::EcsManager& ecs,
        const components::ConveyorComponent& conveyor,
        int lane,
        int slot);

    void placeItemInSlot(
        atlas::scene::EcsManager& ecs,
        components::SequenceComponent& sequence,
        uint8_t sequenceIndex,
        int targetChannel,
        int targetSlot,
        const InsertInfo& info);

    void placeItemInSlot(
        atlas::scene::EcsManager& ecs,
        components::ConveyorComponent& conveyor,
        int targetChannel,
        int targetSlot,
        const InsertInfo& info,
        bool bShouldSetDirectly = false);

    Eigen::Vector2f getSlotPosition(
        const components::SequenceComponent& sequence,
        uint8_t uiSequenceIndex,
        int lane,
        int slot);

    std::optional<ItemInformation> getItemInSlot(
        const components::ConveyorComponent& conveyor,
        int channel,
        int slot);

    std::optional<ItemInformation> getItemInSlot(
        const components::SequenceComponent& sequence,
        uint8_t sequenceIndex,
        int channel,
        int slot);
}
