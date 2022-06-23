#include "ItemPassingUtility.h"
#include <AtlasScene/ECS/Components/EcsManager.h>

#include "ConveyorComponent.h"
#include "ConveyorHelper.h"
#include "DirectionComponent.h"
#include "FactoryComponent.h"
#include "StorageComponent.h"
#include "WorldEntityInformationComponent.h"
#include "AtlasGame/Scene/Components/PositionComponent.h"

namespace
{
    cpp_conv::components::ConveyorComponent::Channel* getTargetChannel(
        const atlas::scene::EcsManager& ecs,
        const cpp_conv::EntityLookupGrid& grid,
        const atlas::scene::EntityId sourceEntity,
        const atlas::scene::EntityId targetEntity,
        cpp_conv::components::ConveyorComponent& targetNode,
        const std::optional<int> sourceChannel)
    {
        using namespace cpp_conv::conveyor_helper;
        using namespace cpp_conv::components;

        if (!ecs.DoesEntityHaveComponents<WorldEntityInformationComponent, DirectionComponent>(sourceEntity))
        {
            return nullptr;
        }

        const auto& [sourceInfo, sourceDirection] = ecs.GetComponents<
            WorldEntityInformationComponent, DirectionComponent>(sourceEntity);
        const auto& [targetPosition, targetDirection] = ecs.GetComponents<atlas::game::scene::components::PositionComponent, DirectionComponent>(
            targetEntity);

        const int mask = static_cast<int>(sourceDirection.m_Direction) | static_cast<int>(targetDirection.m_Direction);
        if (sourceInfo.m_EntityKind != EntityKind::Junction && (mask == 0b11 || (mask >> 2) == 0b11))
        {
            // The nodes are facing either other, no valid path.
            return nullptr;
        }

        if (targetNode.m_bIsCorner)
        {
            RelativeDirection direction;
            if (findNextTailConveyor(ecs, grid, targetPosition.m_Position, targetDirection.m_Direction, direction) ==
                sourceEntity)
            {
                return &targetNode.m_Channels[sourceChannel.value_or(0)];
            }

            return targetNode.m_bIsClockwise
                       ? &targetNode.m_Channels[0]
                       : &targetNode.m_Channels[1];
        }

        if (sourceDirection.m_Direction == targetDirection.m_Direction)
        {
            return &targetNode.m_Channels[sourceChannel.value_or(0)];
        }

        switch (sourceDirection.m_Direction)
        {
        case Direction::Up: return targetDirection.m_Direction == Direction::Left
                                       ? &targetNode.m_Channels[1]
                                       : &targetNode.m_Channels[0];
        case Direction::Down: return targetDirection.m_Direction == Direction::Left
                                         ? &targetNode.m_Channels[0]
                                         : &targetNode.m_Channels[1];
        case Direction::Left: return targetDirection.m_Direction == Direction::Up
                                         ? &targetNode.m_Channels[0]
                                         : &targetNode.m_Channels[1];
        case Direction::Right: return targetDirection.m_Direction == Direction::Up
                                          ? &targetNode.m_Channels[1]
                                          : &targetNode.m_Channels[0];
        }

        return nullptr;
    }

    int getChannelTargetSlot(
        const atlas::scene::EcsManager& ecs,
        const cpp_conv::EntityLookupGrid& grid,
        const atlas::scene::EntityId sourceEntity,
        const atlas::scene::EntityId targetEntity,
        const cpp_conv::components::ConveyorComponent& targetNode,
        const std::optional<int> sourceChannel)
    {
        using namespace cpp_conv::conveyor_helper;
        using namespace cpp_conv::components;

        const auto& [sourceInfo, sourceDirection] = ecs.GetComponents<
            WorldEntityInformationComponent, DirectionComponent>(sourceEntity);
        const auto& [targetPosition, targetDirection] = ecs.GetComponents<atlas::game::scene::components::PositionComponent, DirectionComponent>(
            targetEntity);

        const Direction eSourceDirection = sourceDirection.m_Direction;
        if (eSourceDirection == targetDirection.m_Direction)
        {
            return 0;
        }

        int result = 0;

        RelativeDirection direction;
        if (targetNode.m_bIsCorner && sourceEntity == findNextTailConveyor(
            ecs, grid, targetPosition.m_Position, targetDirection.m_Direction, direction))
        {
            return 0;
        }

        const int sourceChannelValue = sourceChannel.value_or(0);
        switch (eSourceDirection)
        {
        case Direction::Up: result = targetDirection.m_Direction == Direction::Left
                                         ? sourceChannelValue
                                         : c_conveyorChannels - 1 - sourceChannelValue;
            break;
        case Direction::Down: result = targetDirection.m_Direction == Direction::Left
                                           ? c_conveyorChannels - 1 - sourceChannelValue
                                           : sourceChannelValue;
            break;
        case Direction::Left: result = targetDirection.m_Direction == Direction::Up
                                           ? c_conveyorChannels - 1 - sourceChannelValue
                                           : sourceChannelValue;
            break;
        case Direction::Right: result = targetDirection.m_Direction == Direction::Up
                                            ? sourceChannelValue
                                            : c_conveyorChannels - 1 - sourceChannelValue;
            break;
        }

        if (targetNode.m_bIsCorner)
        {
            result++;
        }

        return result;
    }
}

bool tryInsertItemConveyor(
    atlas::scene::EcsManager& ecs,
    const cpp_conv::EntityLookupGrid& grid,
    const atlas::scene::EntityId sourceEntity,
    const atlas::scene::EntityId targetEntity,
    const cpp_conv::ItemId& itemId,
    const std::optional<int> sourceChannel = {},
    const std::optional<Eigen::Vector2f>& sourcePosition = {})
{
    using cpp_conv::components::ConveyorComponent;

    auto& conveyor = ecs.GetComponent<ConveyorComponent>(targetEntity);
    const ConveyorComponent::Channel* pTargetChannel = getTargetChannel(
        ecs,
        grid,
        sourceEntity,
        targetEntity,
        conveyor,
        sourceChannel);
    if (!pTargetChannel)
    {
        return false;
    }

    const int forwardTargetItemSlot = getChannelTargetSlot(ecs, grid, sourceEntity, targetEntity, conveyor,
                                                           sourceChannel);
    if (cpp_conv::conveyor_helper::hasItemInSlot(ecs, conveyor, pTargetChannel->m_ChannelLane, forwardTargetItemSlot))
    {
        return false;
    }

    cpp_conv::conveyor_helper::placeItemInSlot(
        ecs,
        conveyor,
        pTargetChannel->m_ChannelLane,
        forwardTargetItemSlot,
        {
            itemId,
            sourcePosition
        });
    return true;
}

bool tryInsertItemFactory(
    atlas::scene::EcsManager& ecs,
    const cpp_conv::EntityLookupGrid& grid,
    const atlas::scene::EntityId sourceEntity,
    const atlas::scene::EntityId targetEntity,
    const cpp_conv::ItemId& itemId,
    std::optional<int> sourceChannel,
    const std::optional<Eigen::Vector2f>& startPosition)
{
    return false;
}

bool tryInsertItemStorage(
    atlas::scene::EcsManager& ecs,
    const cpp_conv::EntityLookupGrid& grid,
    const atlas::scene::EntityId sourceEntity,
    const atlas::scene::EntityId targetEntity,
    const cpp_conv::ItemId& itemId,
    std::optional<int> sourceChannel,
    const std::optional<Eigen::Vector2f>& startPosition)
{
    auto& storage = ecs.GetComponent<cpp_conv::components::StorageComponent>(targetEntity);
    if (!storage.m_ItemContainer.TryInsert(itemId))
    {
        return false;
    }

    return true;
}

bool cpp_conv::item_passing_utility::entitySupportsInsertion(
    const atlas::scene::EcsManager& ecs,
    const atlas::scene::EntityId targetEntity)
{
    return
        ecs.DoesEntityHaveComponent<components::ConveyorComponent>(targetEntity) ||
        ecs.DoesEntityHaveComponent<components::FactoryComponent>(targetEntity) ||
        ecs.DoesEntityHaveComponent<components::StorageComponent>(targetEntity);
}

bool cpp_conv::item_passing_utility::tryInsertItem(
    atlas::scene::EcsManager& ecs,
    const EntityLookupGrid& grid,
    const atlas::scene::EntityId sourceEntity,
    const atlas::scene::EntityId targetEntity,
    const ItemId itemId,
    const std::optional<int> sourceChannel,
    const std::optional<Eigen::Vector2f> startPosition)
{
    if (ecs.DoesEntityHaveComponent<components::ConveyorComponent>(targetEntity))
    {
        return tryInsertItemConveyor(ecs, grid, sourceEntity, targetEntity, itemId, sourceChannel, startPosition);
    }

    if (ecs.DoesEntityHaveComponent<components::FactoryComponent>(targetEntity))
    {
        return tryInsertItemFactory(ecs, grid, sourceEntity, targetEntity, itemId, sourceChannel, startPosition);
    }

    if (ecs.DoesEntityHaveComponent<components::StorageComponent>(targetEntity))
    {
        return tryInsertItemStorage(ecs, grid, sourceEntity, targetEntity, itemId, sourceChannel, startPosition);
    }

    return false;
}
