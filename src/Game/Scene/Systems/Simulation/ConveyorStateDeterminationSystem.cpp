#include "ConveyorStateDeterminationSystem.h"

#include "Constants.h"
#include "ConveyorComponent.h"
#include "ConveyorHelper.h"
#include "DirectionComponent.h"
#include "Entity.h"
#include "EntityLookupGrid.h"
#include "PositionHelper.h"
#include "WorldEntityInformationComponent.h"
#include "AtlasGame/Scene/Components/PositionComponent.h"
#include "AtlasResource/ResourceLoader.h"
#include "AtlasScene/ECS/Components/EcsManager.h"

namespace
{
    namespace ConveyorVisualState
    {
        enum
        {
            None = 0,
            Corner = 1 << 0,
            Capped = 1 << 1,
            CappedBack = 1 << 2,
            Clockwise = 1 << 3,
        };
    }

    struct ConveyorSlot
    {
        int32_t m_Lane;
        int32_t m_Channel;
    };

    bool isCornerConveyor(
        const atlas::scene::EcsManager& ecs,
        const cpp_conv::EntityLookupGrid& lookupGraph,
        const atlas::game::scene::components::PositionComponent& position,
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
        const atlas::game::scene::components::PositionComponent& position,
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

        return outDirection == RelativeDirection::Left;
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
        const atlas::game::scene::components::PositionComponent& position,
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

    [[nodiscard]] Eigen::Vector2f rotate(const Eigen::Vector2f in, const Rotation rotation, Eigen::Vector2f size)
    {
        const Eigen::Vector2f c_offset(1, 1);
        size -= c_offset;
        switch (rotation)
        {
        case Rotation::Deg90: return {size.y() - in.y(), in.x()};
        case Rotation::Deg180: return {size.x() - in.x(), size.y() - in.y()};
        case Rotation::Deg270: return {size.y() - in.y(), size.x() - in.x()};
        case Rotation::DegZero: break;
        }
        return in;
    }

    Eigen::Vector2f getRenderPosition(
        const atlas::game::scene::components::PositionComponent& positionComponent,
        const cpp_conv::components::DirectionComponent& directionComponent,
        const cpp_conv::components::ConveyorComponent& conveyor,
        const ConveyorSlot slot,
        const bool bAnimate = false,
        const float fLerpFactor = 1.0f,
        const Eigen::Vector2f previousPosition = {})
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

        constexpr float c_firstSlot = -0.25f;
        constexpr float c_secondSlot = 0.25f;
        constexpr float c_lowCornerSlot = -0.1f;
        constexpr float c_highCornerSlot = 0.1f;

        Eigen::Vector2f position;
        if (conveyor.m_bIsCorner)
        {
            if (conveyor.m_bIsClockwise)
            {
                if (slot.m_Lane == 0)
                {
                    switch (slot.m_Channel)
                    {
                    case 0: position = {c_firstSlot, c_secondSlot};
                        break;
                    case 1: position = {c_lowCornerSlot, c_lowCornerSlot};
                        break;
                    case 2: position = {c_secondSlot, c_firstSlot };
                        break;
                    default: ;
                    }
                }
                else
                {
                    position = {c_secondSlot, c_secondSlot};
                }
            }
            else
            {
                if (slot.m_Lane == 0)
                {
                    position = {c_secondSlot, c_firstSlot };
                }
                else
                {
                    switch (slot.m_Channel)
                    {
                    case 0: position = {c_firstSlot,  c_firstSlot };
                        break;
                    case 1: position = {c_lowCornerSlot, c_highCornerSlot};
                        break;
                    case 2: position = {c_secondSlot, c_secondSlot};
                        break;
                    default: ;
                    }
                }
            }
        }
        else
        {
            position = { c_firstSlot + 0.5f * slot.m_Channel, c_firstSlot + 0.5f * slot.m_Lane };
        }

        const Eigen::Vector2f blockSize(1.0f, 1.0f );
        const auto backToOrigin = static_cast<Rotation>((4 - stepsRequired) % 4);
        position = rotate(position, backToOrigin, blockSize);

        return
        {
            positionComponent.m_Position.x() + position.x(),
            positionComponent.m_Position.z() + position.y(),
        };
    }
}

cpp_conv::ConveyorStateDeterminationSystem::ConveyorStateDeterminationSystem(EntityLookupGrid& lookupGrid): m_LookupGrid{lookupGrid}
{
}

void cpp_conv::ConveyorStateDeterminationSystem::Initialise(atlas::scene::EcsManager& ecs)
{
    using namespace components;
    using atlas::scene::EntityId;

    const auto conveyorEntities = ecs.GetEntitiesWithComponents<
        WorldEntityInformationComponent, atlas::game::scene::components::PositionComponent, DirectionComponent, ConveyorComponent>();

    std::vector<EntityId> toAddIndividualTag;
    std::vector<EntityId> toRemoveIndividualTag;
    for (const auto entity : conveyorEntities)
    {
        // Good job this doesn't run frequently...
        const auto& [info, position, direction, conveyor] = ecs.GetComponents<
            WorldEntityInformationComponent, atlas::game::scene::components::PositionComponent, DirectionComponent, ConveyorComponent>(entity);
        const EntityId forwardEntity = m_LookupGrid.GetEntity(position_helper::getForwardPosition(position.m_Position, direction.m_Direction));
        const EntityId backwardsEntity = m_LookupGrid.GetEntity(position_helper::getBackwardsPosition(position.m_Position, direction.m_Direction));

        conveyor.m_bIsCorner = isCornerConveyor(ecs, m_LookupGrid, position, direction);
        conveyor.m_bIsClockwise = conveyor.m_bIsCorner && isClockwiseCorner(ecs, m_LookupGrid, position, direction);

        bool bIsCapped = true;
        if (forwardEntity.IsValid() && ecs.DoesEntityHaveComponent<WorldEntityInformationComponent>(forwardEntity))
        {
            const auto& neighbourInfo = ecs.GetComponent<WorldEntityInformationComponent>(forwardEntity);
            bIsCapped = !isInsertableEntity(neighbourInfo.m_EntityKind);
        }

        bool bIsBackCapped = !conveyor.m_bIsCorner;
        if (!conveyor.m_bIsCorner && backwardsEntity.IsValid() && ecs.DoesEntityHaveComponent<WorldEntityInformationComponent>(backwardsEntity))
        {
            const auto& neighbourInfo = ecs.GetComponent<WorldEntityInformationComponent>(backwardsEntity);
            bIsBackCapped = !isInsertableEntity(neighbourInfo.m_EntityKind);
        }

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

        const bool bAlreadyHasIndividual = ecs.DoesEntityHaveComponent<IndividuallyProcessableConveyorComponent>(entity);
        if (conveyor.m_bIsCorner && !bAlreadyHasIndividual)
        {
            ecs.AddComponent<IndividuallyProcessableConveyorComponent>(entity);
        }
        else if (!conveyor.m_bIsCorner && bAlreadyHasIndividual)
        {
            ecs.RemoveComponent<IndividuallyProcessableConveyorComponent>(entity);
        }
    }
}

void cpp_conv::ConveyorStateDeterminationSystem::Update(atlas::scene::EcsManager&)
{
}
