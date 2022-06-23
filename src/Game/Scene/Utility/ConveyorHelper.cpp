#include "ConveyorHelper.h"

#include "DirectionComponent.h"
#include "WorldEntityInformationComponent.h"

#include <AtlasScene/ECS/Components/EcsManager.h>

#include "EntityLookupGrid.h"
#include "FactoryComponent.h"
#include "PositionHelper.h"
#include "AtlasGame/Scene/Components/PositionComponent.h"

atlas::scene::EntityId cpp_conv::conveyor_helper::findNextTailConveyor(
    const atlas::scene::EcsManager& ecs,
    const EntityLookupGrid& grid,
    const Eigen::Vector3i position,
    const Direction direction,
    RelativeDirection& outDirection)
{
    using namespace components;
    using atlas::scene::EntityId;

    static RelativeDirection directionPriority[] =
    {
        RelativeDirection::Backwards,
        RelativeDirection::Right,
        RelativeDirection::Left,
    };

    Eigen::Vector3i vPositions[4];
    vPositions[static_cast<int>(RelativeDirection::Backwards)] = position_helper::getBackwardsPosition(position, direction);
    vPositions[static_cast<int>(RelativeDirection::Right)] = position_helper::getRightPosition(position, direction);
    vPositions[static_cast<int>(RelativeDirection::Left)] = position_helper::getLeftPosition(position, direction);

    EntityId pTargetConveyor = EntityId::Invalid();
    for (auto d : directionPriority)
    {
        EntityId directionEntity = grid.GetEntity(vPositions[static_cast<int>(d)]);
        if (directionEntity.IsInvalid() || !ecs.DoesEntityHaveComponents<
            atlas::game::scene::components::PositionComponent, DirectionComponent, WorldEntityInformationComponent>(directionEntity))
        {
            continue;
        }

        const auto& info = ecs.GetComponent<WorldEntityInformationComponent>(directionEntity);
        switch (info.m_EntityKind)
        {
        case EntityKind::Stairs:
        case EntityKind::Tunnel:
        case EntityKind::Conveyor:
            {
                auto [targetPosition, targetDirection] = ecs.GetComponents<atlas::game::scene::components::PositionComponent, DirectionComponent>(
                    directionEntity);
                if (position_helper::getForwardPosition(targetPosition.m_Position, targetDirection.m_Direction) == position)
                {
                    pTargetConveyor = directionEntity;
                }
                break;
            }
        case EntityKind::Junction:
            pTargetConveyor = directionEntity;
            break;
        case EntityKind::Producer:
            {
                const auto& [targetPosition, targetDirection, targetFactory] = ecs.GetComponents<
                    atlas::game::scene::components::PositionComponent, DirectionComponent, FactoryComponent>(directionEntity);
                if (targetFactory.m_OutputPipe.has_value() && vPositions[static_cast<int>(direction)] == targetPosition.
                    m_Position + targetFactory.m_OutputPipe.value())
                {
                    pTargetConveyor = directionEntity;
                }
                break;
            }
        case EntityKind::Inserter:
        case EntityKind::Storage:
        default:
            continue;
        case EntityKind::MAX:
            break;
        }

        if (!pTargetConveyor.IsInvalid())
        {
            outDirection = d;
            break;
        }
    }

    return pTargetConveyor;
}

bool cpp_conv::conveyor_helper::hasItemInSlot(const components::SequenceComponent& sequence,
    const uint8_t sequenceIndex, const int channel, const int slot)
{
    const uint64_t slotOffset = sequence.m_Length * 2 - sequenceIndex * 2 - slot - 1;
    const uint64_t uiSetMask = 1ULL << slotOffset;
    return
        (sequence.m_RealizedStates[channel].m_Lanes & uiSetMask) != 0 ||
        (sequence.m_PendingStates[channel].m_PendingMoves & uiSetMask) != 0 ||
        (sequence.m_PendingStates[channel].m_PendingInsertions & uiSetMask) != 0;
}

bool cpp_conv::conveyor_helper::hasItemInSlot(const atlas::scene::EcsManager& ecs,
    const components::ConveyorComponent& conveyor, const int lane, const int slot)
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

void cpp_conv::conveyor_helper::placeItemInSlot(atlas::scene::EcsManager& ecs, components::SequenceComponent& sequence,
    const uint8_t sequenceIndex, const int targetChannel, const int targetSlot, const InsertInfo& info)
{
    components::SequenceComponent::PendingState& pendingState = sequence.m_PendingStates[targetChannel];

    const uint64_t uiSetMask = 1ULL << (sequence.m_Length * 2 - sequenceIndex * 2 - targetSlot - 1);
    assert((pendingState.m_PendingMoves & uiSetMask) == 0);
    assert((pendingState.m_PendingInsertions & uiSetMask) == 0);

    if (sequence.m_RealizedStates[targetChannel].m_Items.GetSize() == 63)
    {
        int i = 0;
        i++;
    }

    pendingState.m_PendingInsertions |= uiSetMask;
    pendingState.m_PendingMoves |= uiSetMask;

    // Gets all bits below the current insertion point
    const uint64_t uiPreviousInsertMask = pendingState.m_PendingInsertions & (uiSetMask - 1);
    const int uiPreviousCount = std::popcount(uiPreviousInsertMask);
    pendingState.m_NewItems.Insert(uiPreviousCount, {info.m_Item, info.m_OriginPosition});
}

void cpp_conv::conveyor_helper::placeItemInSlot(atlas::scene::EcsManager& ecs, components::ConveyorComponent& conveyor,
    const int targetChannel, const int targetSlot, const InsertInfo& info, const bool bShouldSetDirectly)
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
        info.m_OriginPosition.has_value()
    };
}

Eigen::Vector2f cpp_conv::conveyor_helper::getSlotPosition(
    const cpp_conv::components::SequenceComponent& sequence,
    const uint8_t uiSequenceIndex, const int lane, const int slot)
{
    const Eigen::Vector2f visual = sequence.m_LaneVisualOffsets[lane];
    const Eigen::Vector2f unitDirection2d{sequence.m_UnitDirection.x(), sequence.m_UnitDirection.y()};
    return visual + unitDirection2d * uiSequenceIndex + unitDirection2d * (0.5f * slot);
}

std::optional<cpp_conv::conveyor_helper::ItemInformation> cpp_conv::conveyor_helper::getItemInSlot(
    const components::ConveyorComponent& conveyor,
    const int channel,
    const int slot)
{
    const auto& rTargetChannel = conveyor.m_Channels[channel];
    const auto& forwardTargetItem = rTargetChannel.m_pSlots[slot].m_Item;

    if (forwardTargetItem.m_Item.IsEmpty())
    {
        return {};
    }

    if (forwardTargetItem.m_bShouldAnimate && forwardTargetItem.m_PreviousPosition.has_value())
    {
        return {{
            forwardTargetItem.m_Item,
            forwardTargetItem.m_PreviousPosition.value(),
            forwardTargetItem.m_bShouldAnimate
        }};
    }

    return {{
        forwardTargetItem.m_Item,
        rTargetChannel.m_pSlots[slot].m_VisualPosition,
        forwardTargetItem.m_bShouldAnimate
    }};
}

std::optional<cpp_conv::conveyor_helper::ItemInformation> cpp_conv::conveyor_helper::getItemInSlot(
    const components::SequenceComponent& sequence,
    const uint8_t sequenceIndex,
    const int channel,
    const int slot)
{
    const uint64_t slotOffset = sequence.m_Length * 2 - sequenceIndex * 2 - slot - 1;
    const uint64_t uiSetMask = 1ULL << slotOffset;
    const bool bHasObtainableItem = (sequence.m_RealizedStates[channel].m_Lanes & uiSetMask) != 0;
    if (!bHasObtainableItem)
    {
        return {};
    }

    const components::SequenceComponent::RealizedState& realizedState = sequence.m_RealizedStates[channel];
    const uint64_t uiPreviousInsertMask = realizedState.m_Lanes & (uiSetMask - 1);
    const int uiPreviousCount = std::popcount(uiPreviousInsertMask);
    const components::SequenceComponent::SlotItem item = realizedState.m_Items.Peek(uiPreviousCount);

    if (!item.m_Item.IsValid())
    {
        return {};
    }

    const bool bDidItemMostLastFrame = ((realizedState.m_RealizedMovements >> slotOffset) & 0b1) == 1;
    const bool bHasOverridePosition = ((realizedState.m_HasOverridePosition >> slotOffset) & 0b1) == 1;

    if (bHasOverridePosition)
    {
        return {{
            item.m_Item,
            item.m_Position.value(),
            bDidItemMostLastFrame
        }};
    }

    return {{
        item.m_Item,
        getSlotPosition(sequence, sequenceIndex, channel, slot - 1),
        bDidItemMostLastFrame
    }};
}
