#include "SequenceFormationSystem.h"

#include "ConveyorComponent.h"
#include "DirectionComponent.h"
#include "EntityGrid.h"
#include "FactoryComponent.h"
#include "PositionComponent.h"
#include "SequenceComponent.h"
#include "vector_set.h"
#include "WorldEntityInformationComponent.h"

namespace
{
    atlas::scene::EntityId findNextTailConveyor(
        atlas::scene::EcsManager& ecs,
        cpp_conv::EntityLookupGrid& grid,
        Eigen::Vector3i position,
        Direction direction,
        RelativeDirection& outDirection)
    {
        using namespace cpp_conv::components;
        using atlas::scene::EntityId;

        static RelativeDirection directionPriority[] =
        {
            RelativeDirection::Backwards,
            RelativeDirection::Right,
            RelativeDirection::Left,
        };

        Eigen::Vector3i vPositions[4];
        vPositions[static_cast<int>(RelativeDirection::Backwards)] = cpp_conv::grid::getBackwardsPosition(position, direction);
        vPositions[static_cast<int>(RelativeDirection::Right)] = cpp_conv::grid::getRightPosition(position, direction);
        vPositions[static_cast<int>(RelativeDirection::Left)] = cpp_conv::grid::getLeftPosition(position, direction);

        EntityId pTargetConveyor = EntityId::Invalid();
        for (auto d : directionPriority)
        {
            EntityId directionEntity = grid.GetEntity(vPositions[static_cast<int>(d)]);
            if (directionEntity.IsInvalid() || !ecs.DoesEntityHaveComponents<PositionComponent, DirectionComponent, WorldEntityInformationComponent>(directionEntity))
            {
                continue;
            }

            auto& info = ecs.GetComponent<WorldEntityInformationComponent>(directionEntity);
            switch (info.m_EntityKind)
            {
                case EntityKind::Stairs:
                case EntityKind::Tunnel:
                case EntityKind::Conveyor:
                    {
                        auto [targetPosition, targetDirection] = ecs.GetComponents<PositionComponent, DirectionComponent>(directionEntity);
                        if (cpp_conv::grid::getForwardPosition(targetPosition.m_Position, targetDirection.m_Direction) == position)
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
                    const auto& [targetPosition, targetDirection, targetFactory] = ecs.GetComponents<PositionComponent, DirectionComponent, FactoryComponent>(directionEntity);
                    if (targetFactory.m_OutputPipe.has_value() && vPositions[static_cast<int>(direction)] == targetPosition.m_Position + targetFactory.m_OutputPipe.value())
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

    atlas::scene::EntityId traceHeadConveyor(
        const atlas::scene::EcsManager& ecs,
        cpp_conv::EntityLookupGrid& grid,
        atlas::scene::EntityId currentConveyor)
    {
        using namespace cpp_conv::components;
        using atlas::scene::EntityId;

        static RelativeDirection directionPriority[] =
        {
            RelativeDirection::Backwards,
            RelativeDirection::Right,
            RelativeDirection::Left,
        };

        const EntityId searchStart = currentConveyor;

        while (true)
        {
            auto [position, direction] = ecs.GetComponents<PositionComponent, DirectionComponent>(currentConveyor);
            Eigen::Vector3i forwardPosition = cpp_conv::grid::getForwardPosition(position.m_Position, direction.m_Direction);
            atlas::scene::EntityId targetEntity = grid.GetEntity(forwardPosition);

            if (!ecs.DoesEntityHaveComponents<PositionComponent, DirectionComponent, ConveyorComponent>(targetEntity))
            {
                break;
            }

            auto [targetPosition, targetDirection, targetConveyor] = ecs.GetComponents<PositionComponent, DirectionComponent, ConveyorComponent>(targetEntity);
            if (targetConveyor.m_bIsCorner)
            {
                break;
            }

            // Check that the route ahead of us doesn't have another potential chain which would take priority over this
            {
                Eigen::Vector3i vPotentialNeighbours[4];
                vPotentialNeighbours[static_cast<int>(RelativeDirection::Backwards)] = cpp_conv::grid::getBackwardsPosition(targetPosition.m_Position, targetDirection.m_Direction);
                vPotentialNeighbours[static_cast<int>(RelativeDirection::Right)] = cpp_conv::grid::getRightPosition(targetPosition.m_Position, targetDirection.m_Direction);
                vPotentialNeighbours[static_cast<int>(RelativeDirection::Left)] = cpp_conv::grid::getLeftPosition(targetPosition.m_Position, targetDirection.m_Direction);

                for (auto neighbourCell : directionPriority)
                {
                    auto neighbourEntity = grid.GetEntity(vPotentialNeighbours[static_cast<int>(neighbourCell)]);
                    if (neighbourEntity != EntityId::Invalid() || !ecs.DoesEntityHaveComponents<PositionComponent, DirectionComponent, ConveyorComponent>(neighbourEntity))
                    {
                        continue;
                    }

                    if (targetEntity == neighbourEntity)
                    {
                        continue;
                    }

                    const auto& [neighbourPosition, neighbourDirection] = ecs.GetComponents<PositionComponent, DirectionComponent>(neighbourEntity);

                    // Node ahead of has a better candidate, meaning we are the terminus.
                    if (cpp_conv::grid::getForwardPosition(neighbourPosition.m_Position, neighbourDirection.m_Direction) == forwardPosition)
                    {
                        return currentConveyor;
                    }
                }
            }

            if (targetEntity == searchStart)
            {
                break;
            }

            currentConveyor = targetEntity;
        }

        return currentConveyor;
    }

    atlas::scene::EntityId traceTailConveyor(
        atlas::scene::EcsManager& ecs,
        cpp_conv::EntityLookupGrid& grid,
        atlas::scene::EntityId searchStart,
        atlas::scene::EntityId head,
        std::vector<atlas::scene::EntityId>& vOutConveyors)
    {
        using namespace cpp_conv::components;
        using atlas::scene::EntityId;

        EntityId currentConveyor = searchStart;

        while (true)
        {
            const auto& [position, direction] = ecs.GetComponents<PositionComponent, DirectionComponent>(searchStart);
            vOutConveyors.push_back(currentConveyor);

            RelativeDirection tmp;
            const EntityId targetConveyor = findNextTailConveyor(ecs, grid, position.m_Position, direction.m_Direction, tmp);

            if (targetConveyor.IsInvalid() ||
                targetConveyor == searchStart ||
                targetConveyor == head ||
                ecs.DoesEntityHaveComponents<PositionComponent, DirectionComponent, ConveyorComponent>(targetConveyor))
            {
                break;
            }

            auto& targetConveyorComponent = ecs.GetComponent<ConveyorComponent>(targetConveyor);
            if (targetConveyorComponent.m_bIsCorner)
            {
                break;
            }

            currentConveyor = targetConveyor;
        }

        std::ranges::reverse(vOutConveyors);
        return currentConveyor;
    }
}

void cpp_conv::SequenceFormationSystem::Initialise(atlas::scene::EcsManager& ecs)
{
    using namespace components;
    using atlas::scene::EntityId;

    const auto conveyorEntities = ecs.GetEntitiesWithComponents<PositionComponent, DirectionComponent, ConveyorComponent>();
    cpp_conveyor::vector_set<EntityId> alreadyProcessedConveyors(conveyorEntities.size());

    // Remove existing sequences
    for(const auto sequence : ecs.GetEntitiesWithComponents<SequenceComponent>())
    {
        ecs.RemoveEntity(sequence);
    }

    for(auto entity : conveyorEntities)
    {
        if (alreadyProcessedConveyors.contains(entity))
        {
            continue;
        }

        const auto& [position, direction, conveyor] = ecs.GetComponents<PositionComponent, DirectionComponent, ConveyorComponent>(entity);

        if (conveyor.m_bIsCorner)
        {
            conveyor.m_Sequence = EntityId::Invalid();
            continue;
        }

        std::vector<EntityId> vConveyors;
        const EntityId pHeadConveyor = traceHeadConveyor(ecs, m_LookupGrid, entity);
        traceTailConveyor(ecs, m_LookupGrid, pHeadConveyor, pHeadConveyor, vConveyors);

        const auto end = vConveyors.end();
        std::vector<EntityId>::iterator chunk_begin;
        std::vector<EntityId>::iterator chunk_end;
        chunk_end = chunk_begin = vConveyors.begin();
        do
        {
            if (static_cast<uint32_t>(std::distance(chunk_end, end)) < c_MaxSequenceLength)
            {
                chunk_end = end;
            }
            else
            {
                std::advance(chunk_end, c_MaxSequenceLength);
            }

            std::vector vSequenceConveyors(chunk_begin, chunk_end);
            const auto& pTailConveyor = ecs.GetComponent<ConveyorComponent>(vSequenceConveyors.front());

            const EntityId sequenceId = ecs.AddEntity();
            SequenceComponent& component = ecs.AddComponent<SequenceComponent>(
                sequenceId,
                SequenceComponent((uint8_t)vSequenceConveyors.size())
            );
            /*component.m_HeadConveyor = vSequenceConveyors[vSequenceConveyors.size() - 1];
            component.m_LaneOneVisualPosition = pTailConveyor.m_Channels[0].m_pSlots[0].m_VisualPosition;
            component.m_LaneTwoVisualPosition = pTailConveyor.m_Channels[1].m_pSlots[0].m_VisualPosition;
            component.m_UnitDirection = pTailConveyor.m_Channels[0].m_pSlots[1].m_VisualPosition - pTailConveyor.m_Channels[0].m_pSlots[0].m_VisualPosition;
            component.m_MoveTick = pTailConveyor.m_MoveTick;*/

            for(size_t i = 0; i < vSequenceConveyors.size(); ++i)
            {
                ecs.GetComponent<ConveyorComponent>(vSequenceConveyors[i]).m_Sequence = sequenceId;
                alreadyProcessedConveyors.insert(vSequenceConveyors[i]);
            }

            chunk_begin = chunk_end;
        }
        while (std::distance(chunk_begin, end) > 0);
    }
}
