#include "SequenceFormationSystem.h"

#include "ConveyorComponent.h"
#include "ConveyorHelper.h"
#include "DirectionComponent.h"
#include "FactoryComponent.h"
#include "PositionHelper.h"
#include "SequenceComponent.h"
#include "vector_set.h"
#include "AtlasGame/Scene/Components/PositionComponent.h"

namespace
{
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
            auto [position, direction] = ecs.GetComponents<atlas::game::scene::components::PositionComponent, DirectionComponent>(currentConveyor);
            Eigen::Vector3i forwardPosition = cpp_conv::position_helper::getForwardPosition(
                position.m_Position, direction.m_Direction);
            EntityId targetEntity = grid.GetEntity(forwardPosition);

            if (targetEntity.IsInvalid() || !ecs.DoesEntityHaveComponents<
                atlas::game::scene::components::PositionComponent, DirectionComponent, ConveyorComponent>(targetEntity))
            {
                break;
            }

            auto [targetPosition, targetDirection, targetConveyor] = ecs.GetComponents<
                atlas::game::scene::components::PositionComponent, DirectionComponent, ConveyorComponent>(targetEntity);
            if (targetConveyor.m_bIsCorner)
            {
                break;
            }

            // Check that the route ahead of us doesn't have another potential chain which would take priority over this
            {
                Eigen::Vector3i vPotentialNeighbours[4];
                vPotentialNeighbours[static_cast<int>(RelativeDirection::Backwards)] =
                    cpp_conv::position_helper::getBackwardsPosition(targetPosition.m_Position, targetDirection.m_Direction);
                vPotentialNeighbours[static_cast<int>(RelativeDirection::Right)] = cpp_conv::position_helper::getRightPosition(
                    targetPosition.m_Position, targetDirection.m_Direction);
                vPotentialNeighbours[static_cast<int>(RelativeDirection::Left)] = cpp_conv::position_helper::getLeftPosition(
                    targetPosition.m_Position, targetDirection.m_Direction);

                for (auto neighbourCell : directionPriority)
                {
                    auto neighbourEntity = grid.GetEntity(vPotentialNeighbours[static_cast<int>(neighbourCell)]);
                    if (neighbourEntity == EntityId::Invalid() || !ecs.DoesEntityHaveComponents<
                        atlas::game::scene::components::PositionComponent, DirectionComponent, ConveyorComponent>(neighbourEntity))
                    {
                        continue;
                    }

                    if (neighbourEntity == currentConveyor)
                    {
                        break;
                    }

                    const auto& [neighbourPosition, neighbourDirection] = ecs.GetComponents<
                        atlas::game::scene::components::PositionComponent, DirectionComponent>(neighbourEntity);

                    // Node ahead of has a better candidate, meaning we are the terminus.
                    if (cpp_conv::position_helper::getForwardPosition(neighbourPosition.m_Position, neighbourDirection.m_Direction)
                        == forwardPosition)
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
            const auto& [position, direction] = ecs.GetComponents<atlas::game::scene::components::PositionComponent, DirectionComponent>(
                currentConveyor);
            vOutConveyors.push_back(currentConveyor);

            RelativeDirection tmp;
            const EntityId targetConveyor = cpp_conv::conveyor_helper::findNextTailConveyor(
                ecs, grid, position.m_Position, direction.m_Direction, tmp);

            if (targetConveyor.IsInvalid() ||
                targetConveyor == searchStart ||
                targetConveyor == head ||
                !ecs.DoesEntityHaveComponents<atlas::game::scene::components::PositionComponent, DirectionComponent, ConveyorComponent>(targetConveyor))
            {
                break;
            }

            const auto& targetConveyorComponent = ecs.GetComponent<ConveyorComponent>(targetConveyor);
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

cpp_conv::SequenceFormationSystem::SequenceFormationSystem(EntityLookupGrid& lookupGrid): m_LookupGrid{lookupGrid}
{
}

void cpp_conv::SequenceFormationSystem::Initialise(atlas::scene::EcsManager& ecs)
{
    using namespace components;
    using atlas::scene::EntityId;

    const auto conveyorEntities = ecs.GetEntitiesWithComponents<
        atlas::game::scene::components::PositionComponent, DirectionComponent, ConveyorComponent>();
    cpp_conveyor::vector_set<EntityId> alreadyProcessedConveyors(conveyorEntities.size());

    // Remove existing sequences
    for (const auto sequence : ecs.GetEntitiesWithComponents<SequenceComponent>())
    {
        ecs.RemoveEntity(sequence);
    }

    for (auto entity : conveyorEntities)
    {
        if (alreadyProcessedConveyors.contains(entity))
        {
            continue;
        }

        const auto& [position, direction, conveyor] = ecs.GetComponents<
            atlas::game::scene::components::PositionComponent, DirectionComponent, ConveyorComponent>(entity);

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
            const auto unitDirection2d =
                pTailConveyor.m_Channels[0].m_pSlots[1].m_VisualPosition - pTailConveyor.m_Channels[0].m_pSlots[0].
                m_VisualPosition;
            const auto normalizedUnitDirection2d = unitDirection2d.normalized();

            const EntityId sequenceId = ecs.AddEntity();
            SequenceComponent& component = ecs.AddComponent<SequenceComponent>(
                sequenceId,
                static_cast<uint8_t>(vSequenceConveyors.size()),
                vSequenceConveyors[vSequenceConveyors.size() - 1],
                pTailConveyor.m_Channels[0].m_pSlots[0].m_VisualPosition,
                pTailConveyor.m_Channels[1].m_pSlots[0].m_VisualPosition,
                Eigen::Vector3f(normalizedUnitDirection2d.x(), normalizedUnitDirection2d.y(), 0.0f),
                pTailConveyor.m_MoveTick
            );

            for (size_t i = 0; i < vSequenceConveyors.size(); ++i)
            {
                auto& localConveyor = ecs.GetComponent<ConveyorComponent>(vSequenceConveyors[i]);
                localConveyor.m_Sequence = sequenceId;
                localConveyor.m_SequenceIndex = static_cast<uint8_t>(i);
                alreadyProcessedConveyors.insert(vSequenceConveyors[i]);
            }

            chunk_begin = chunk_end;
        }
        while (std::distance(chunk_begin, end) > 0);
    }
}

void cpp_conv::SequenceFormationSystem::Update(atlas::scene::EcsManager&)
{
}
