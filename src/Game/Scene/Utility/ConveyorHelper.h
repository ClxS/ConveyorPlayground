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
        std::optional<uint8_t> m_SourceChannel{};
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
        const int slot)
    {
        const int uiSetMask = 1ULL << (sequence.m_Length * 2 - sequenceIndex * 2 - slot - 1);
        return
            (sequence.m_RealizedStates[channel].m_Lanes & uiSetMask) != 0 ||
            (sequence.m_PendingStates[channel].m_PendingMoves & uiSetMask) != 0 ||
            (sequence.m_PendingStates[channel].m_PendingInsertions & uiSetMask) != 0;
    }

    inline bool hasItemInSlot(
        const atlas::scene::EcsManager& ecs,
        const components::ConveyorComponent& conveyor,
        const int lane,
        const int slot)
    {
        if (conveyor.m_Sequence.IsValid())
        {
            const auto& sequence = ecs.GetComponent<components::SequenceComponent>(conveyor.m_Sequence);
            return hasItemInSlot(sequence, conveyor.m_SequenceIndex, lane, slot);
        }

        const components::ConveyorComponent::Channel& rTargetChannel = conveyor.m_Channels[lane];
        const components::ConveyorComponent::PlacedItem& forwardTargetItem = rTargetChannel.m_pSlots[slot].m_Item;
        const components::ConveyorComponent::PlacedItem& forwardPendingItem = rTargetChannel.m_pPendingItems[slot];
        return (!forwardTargetItem.m_Item.IsEmpty() || !forwardPendingItem.m_Item.IsEmpty());
    }

    inline void placeItemInSlot(
        atlas::scene::EcsManager& ecs,
        components::SequenceComponent& sequence,
        const uint8_t sequenceIndex,
        const int targetChannel,
        const int targetSlot,
        const InsertInfo& info)
    {
        components::SequenceComponent::PendingState& pendingState = sequence.m_PendingStates[targetChannel];

        const uint64_t uiSetMask = 1ULL << (sequence.m_Length * 2 - sequenceIndex * 2 - targetSlot - 1);
        assert((pendingState.m_PendingMoves & uiSetMask) == 0);
        assert((pendingState.m_PendingInsertions & uiSetMask) == 0);

        pendingState.m_PendingInsertions |= uiSetMask;
        pendingState.m_PendingMoves |= uiSetMask;

        // Gets all bits below the current insertion point
        const uint64_t uiPreviousInsertMask = pendingState.m_PendingInsertions & (uiSetMask - 1);
        const int uiPreviousCount = std::popcount(uiPreviousInsertMask);
        pendingState.m_NewItems.Insert(uiPreviousCount, {info.m_Item, info.m_OriginPosition});
    }

    inline void placeItemInSlot(
        atlas::scene::EcsManager& ecs,
        components::ConveyorComponent& conveyor,
        const int targetChannel,
        const int targetSlot,
        const InsertInfo& info,
        const bool bShouldSetDirectly = false)
    {
        assert(!hasItemInSlot(ecs, conveyor, targetChannel, targetSlot));

        if (conveyor.m_Sequence.IsValid())
        {
            auto& sequence = ecs.GetComponent<components::SequenceComponent>(conveyor.m_Sequence);
            placeItemInSlot(ecs, sequence, conveyor.m_SequenceIndex, targetChannel, targetSlot, info);
            return;
        }

        components::ConveyorComponent::PlacedItem& forwardTargetItem =
        (bShouldSetDirectly
             ? conveyor.m_Channels[targetChannel].m_pSlots[targetSlot].m_Item
             : conveyor.m_Channels[targetChannel].m_pPendingItems[targetSlot]);

        forwardTargetItem =
        {
            info.m_Item,
            info.m_OriginPosition,
        };
    }
}
