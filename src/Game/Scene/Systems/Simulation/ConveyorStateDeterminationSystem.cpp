#include "ConveyorStateDeterminationSystem.h"

#include "ConveyorComponent.h"
#include "ConveyorHelper.h"
#include "DirectionComponent.h"
#include "Entity.h"
#include "EntityGrid.h"
#include "EntityLookupGrid.h"
#include "PositionComponent.h"
#include "ResourceManager.h"
#include "SpriteLayerComponent.h"
#include "TargetingUtility.h"
#include "WorldEntityInformationComponent.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

namespace
{
    struct ConveyorSlot
    {
        int32_t m_Lane;
        int32_t m_Channel;
    };

    bool isCornerConveyor(
        const atlas::scene::EcsManager& ecs,
        const cpp_conv::EntityLookupGrid& lookupGraph,
        const cpp_conv::components::PositionComponent& position,
        const cpp_conv::components::DirectionComponent& direction)
    {
        RelativeDirection outDirection;
        const atlas::scene::EntityId tailConverter = cpp_conv::conveyor_helper::findNextTailConveyor(
            ecs, lookupGraph, position.m_Position, direction.m_Direction, outDirection);
        if (tailConverter.IsInvalid() || outDirection == RelativeDirection::Backwards || outDirection ==
            RelativeDirection::Forward)
        {
            return false;
        }

        const auto& [otherInfo, otherDirection]
            = ecs.GetComponents<cpp_conv::components::WorldEntityInformationComponent,
                                cpp_conv::components::DirectionComponent>(tailConverter);
        return otherDirection.m_Direction != direction.m_Direction || otherInfo.m_EntityKind == EntityKind::Junction;
    }

    bool isClockwiseCorner(
        const atlas::scene::EcsManager& ecs,
        const cpp_conv::EntityLookupGrid& lookupGraph,
        const cpp_conv::components::PositionComponent& position,
        const cpp_conv::components::DirectionComponent& direction)
    {
        RelativeDirection outDirection;
        const atlas::scene::EntityId tailConverter = cpp_conv::conveyor_helper::findNextTailConveyor(
            ecs, lookupGraph, position.m_Position, direction.m_Direction, outDirection);
        if (tailConverter.IsInvalid() || outDirection == RelativeDirection::Backwards || outDirection ==
            RelativeDirection::Forward)
        {
            return false;
        }

        return (outDirection == RelativeDirection::Left) || (outDirection == RelativeDirection::Right);
    }

    bool isInsertableEntity(const EntityKind kind)
    {
        return
            kind == EntityKind::Conveyor ||
            kind == EntityKind::Junction ||
            kind == EntityKind::Stairs ||
            kind == EntityKind::Storage ||
            kind == EntityKind::Tunnel;
    }

    std::tuple<int, Direction> getInnerMostCornerChannel(
        const atlas::scene::EcsManager& ecs,
        const cpp_conv::EntityLookupGrid& lookupGraph,
        const cpp_conv::components::PositionComponent& position,
        const cpp_conv::components::DirectionComponent& direction)
    {
        RelativeDirection outDirection;
        const atlas::scene::EntityId tailConverter = cpp_conv::conveyor_helper::findNextTailConveyor(
            ecs, lookupGraph, position.m_Position, direction.m_Direction, outDirection);
        if (tailConverter.IsInvalid() || outDirection == RelativeDirection::Backwards || outDirection ==
            RelativeDirection::Forward)
        {
            return std::make_tuple(-1, Direction::Up);
        }

        const auto& otherDirection = ecs.GetComponent<cpp_conv::components::DirectionComponent>(tailConverter);

        Direction selfDirection = direction.m_Direction;
        Direction backDirection = otherDirection.m_Direction;
        while (selfDirection != Direction::Up)
        {
            selfDirection = cpp_conv::direction::rotate90DegreeClockwise(selfDirection);
            backDirection = cpp_conv::direction::rotate90DegreeClockwise(backDirection);
        }

        return std::make_tuple(backDirection == Direction::Right ? 1 : 0, otherDirection.m_Direction);
    }

    Eigen::Vector2f getRenderPosition(
        const cpp_conv::components::PositionComponent& positionComponent,
        const cpp_conv::components::DirectionComponent& directionComponent,
        const cpp_conv::components::ConveyorComponent& conveyor,
        const ConveyorSlot slot,
        const bool bAnimate = false,
        const float fLerpFactor = 1.0f,
        const Vector2F previousPosition = {})
    {
        // This method translates the current direction in Right-facing space, determines the offsets, then rotates the offsets back to their original
        // direction-facing space.

        Direction eDirection = directionComponent.m_Direction;
        int stepsRequired = 0;
        while (eDirection != Direction::Right)
        {
            eDirection = cpp_conv::direction::rotate90DegreeClockwise(eDirection);
            stepsRequired++;
        }

        Vector2F position;
        if (conveyor.m_bIsCorner)
        {
            if (conveyor.m_bIsClockwise)
            {
                if (slot.m_Lane == 0)
                {
                    switch (slot.m_Channel)
                    {
                    case 0: position = {1.0f, 2.0f};
                        break;
                    case 1: position = {1.2f, 1.2f};
                        break;
                    case 2: position = {2.0f, 1.0f};
                        break;
                    }
                }
                else
                {
                    position = {2.0f, 2.0f};
                }
            }
            else
            {
                if (slot.m_Lane == 0)
                {
                    position = {2.0f, 1.0f};
                }
                else
                {
                    switch (slot.m_Channel)
                    {
                    case 0: position = {1.0f, 1.0f};
                        break;
                    case 1: position = {1.2f, 1.8f};
                        break;
                    case 2: position = {2.0f, 2.0f};
                        break;
                    }
                }
            }
        }
        else
        {
            position = {1.0f + static_cast<float>(slot.m_Channel), 1.0f + static_cast<float>(slot.m_Lane)};
        }

        constexpr float c_fBlockSize = 4;
        const Vector2F blockSize(c_fBlockSize, c_fBlockSize);
        const auto backToOrigin = static_cast<Rotation>((4 - stepsRequired) % 4);
        position = position.Rotate(backToOrigin, blockSize);

        const Vector2F offset = position * 0.5f * c_fBlockSize - Vector2F(1.0f, 1.0f) - (cpp_conv::renderer::c_gridScale
            / c_fBlockSize);

        const Vector2F end = (Vector2F(static_cast<float>(positionComponent.m_Position.x()),
                                       static_cast<float>(positionComponent.m_Position.y())) * blockSize) + offset;
        if (!bAnimate)
        {
            return Eigen::Vector2f(end.GetX(), end.GetY());
        }

        const auto outValue = previousPosition + ((end - previousPosition) * fLerpFactor);
        return {outValue.GetX(), outValue.GetY()};
    }
}

void cpp_conv::ConveyorStateDeterminationSystem::Initialise(atlas::scene::EcsManager& ecs)
{
    using namespace components;
    using atlas::scene::EntityId;

    const auto conveyorEntities = ecs.GetEntitiesWithComponents<
        WorldEntityInformationComponent, PositionComponent, DirectionComponent, ConveyorComponent>();

    for (const auto entity : conveyorEntities)
    {
        // Good job this doesn't run frequently...
        const auto& [info, position, direction, conveyor] = ecs.GetComponents<
            WorldEntityInformationComponent, PositionComponent, DirectionComponent, ConveyorComponent>(entity);
        const EntityId pEntity = m_LookupGrid.GetEntity(
            grid::getForwardPosition(position.m_Position, direction.m_Direction));

        conveyor.m_bIsCorner = isCornerConveyor(ecs, m_LookupGrid, position, direction);
        conveyor.m_bIsClockwise = isClockwiseCorner(ecs, m_LookupGrid, position, direction);
        conveyor.m_bIsCapped = pEntity.IsInvalid() || !isInsertableEntity(info.m_EntityKind);

        std::tie(conveyor.m_InnerMostChannel, conveyor.m_CornerDirection) = getInnerMostCornerChannel(
            ecs, m_LookupGrid, position, direction);

        for (auto iLane = 0; iLane < conveyor.m_Channels.size(); iLane++)
        {
            conveyor.m_Channels[iLane].m_ChannelLane = iLane;
            conveyor.m_Channels[iLane].m_LaneLength = 2;
            if (conveyor.m_bIsCorner)
            {
                conveyor.m_Channels[iLane].m_LaneLength += conveyor.m_InnerMostChannel == iLane ? -1 : 1;
            }

            for (auto iSlot = 0; iSlot < conveyor.m_Channels[iLane].m_pSlots.size(); iSlot++)
            {
                conveyor.m_Channels[iLane].m_pSlots[iSlot].m_VisualPosition = getRenderPosition(
                    position, direction, conveyor, {iLane, iSlot});
            }
        }

        if (ecs.DoesEntityHaveComponent<SpriteLayerComponent<1>>(entity))
        {
            auto& sprite = ecs.GetComponent<SpriteLayerComponent<1>>(entity);
            if (!conveyor.m_bIsCorner)
            {
                if (!conveyor.m_bIsCapped)
                {
                    sprite.m_pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(
                        resources::registry::assets::conveyors::c_ConveyorStraight);
                }
                else
                {
                    sprite.m_pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(
                        resources::registry::assets::conveyors::c_ConveyorStraightEnd);
                }
            }
            else
            {
                if (conveyor.m_bIsClockwise)
                {
                    sprite.m_pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(
                        resources::registry::assets::conveyors::c_ConveyorCornerClockwise);
                }
                else
                {
                    sprite.m_pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(
                        resources::registry::assets::conveyors::c_ConveyorCornerAntiClockwise);
                }
            }
        }
    }
}
