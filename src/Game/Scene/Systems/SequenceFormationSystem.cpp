#include "SequenceFormationSystem.h"

#include "ConveyorComponent.h"
#include "DirectionComponent.h"
#include "EntityGrid.h"
#include "PositionComponent.h"
#include "SequenceComponent.h"
#include "vector_set.h"

namespace
{
    atlas::scene::EntityId traceHeadConveyor(
        const atlas::scene::EcsManager& ecs,
        cpp_conv::EntityLookupGrid& grid,
        const cpp_conv::components::PositionComponent& position,
        const cpp_conv::components::DirectionComponent& direction,
        const cpp_conv::components::ConveyorComponent& conveyor)
    {
        static RelativeDirection directionPriority[] =
        {
            RelativeDirection::Backwards,
            RelativeDirection::Right,
            RelativeDirection::Left,
        };


        while (true)
        {
            Eigen::Vector3i forwardPosition = cpp_conv::grid::getForwardPosition(position.m_Position, direction.m_Direction);
            atlas::scene::EntityId targetEntity = grid.GetEntity(forwardPosition);

            if (!ecs.DoesEntityHaveComponent<cpp_conv::components::ConveyorComponent>(targetEntity))
            {
                break;
            }

            auto& targetConveyor = ecs.GetComponent<cpp_conv::components::ConveyorComponent>(targetEntity);
            if (targetConveyor.m_bIsCorner)
            {
                break;
            }

            const Eigen::Vector3i vPotentialNeighbours[4];
            vPotentialNeighbours[static_cast<int>(RelativeDirection::Backwards)] = cpp_conv::grid::getBackwardsPosition(*targetEntity);
            vPotentialNeighbours[static_cast<int>(RelativeDirection::Right)] = cpp_conv::grid::getRightPosition(*targetEntity);
            vPotentialNeighbours[static_cast<int>(RelativeDirection::Left)] = cpp_conv::grid::getLeftPosition(*targetEntity);

            for (auto neighbourCell : directionPriority)
            {
                auto neighbourEntity = grid.GetEntity(neighbourCell);


                const Eigen::Vector3i* pDirectionEntity = vPotentialNeighbours[static_cast<int>(direction)];
                if (pDirectionEntity == nullptr || pDirectionEntity->m_eEntityKind != EntityKind::Conveyor)
                {
                    continue;
                }

                if (pDirectionEntity == pCurrentConveyor)
                {
                    continue;
                }

                if (grid::getForwardPosition(*pDirectionEntity) == forwardPosition)
                {
                    return pCurrentConveyor;
                }
            }

            if (targetEntity == &searchStart || targetEntity->IsCorner())
            {
                break;
            }

            pCurrentConveyor = static_cast<Conveyor*>(targetEntity);
        }

        return pCurrentConveyor;
    }

    EntityId traceTailConveyor(WorldMap& map, const Conveyor& searchStart, const Conveyor& head, std::vector<Conveyor*>& vOutConveyors)
    {
        auto pCurrentConveyor = map.GetEntity<Conveyor>(searchStart.m_position, EntityKind::Conveyor);

        assert(pCurrentConveyor != nullptr); // If it is null here, we are misusing this method.

        while (true)
        {
            vOutConveyors.push_back(pCurrentConveyor);

            RelativeDirection direction;
            Entity* pTargetConveyor = targeting_util::findNextTailConveyor(map, *pCurrentConveyor, direction);
            if (!pTargetConveyor ||
                pTargetConveyor->m_eEntityKind != EntityKind::Conveyor ||
                static_cast<Conveyor*>(pTargetConveyor)->IsCorner() ||
                pTargetConveyor == &searchStart ||
                pTargetConveyor == &head)
            {
                break;
            }

            pCurrentConveyor = static_cast<Conveyor*>(pTargetConveyor);
        }

        std::ranges::reverse(vOutConveyors);
        return pCurrentConveyor;
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

        auto& [position, direction, conveyor] = ecs.GetComponents<PositionComponent, DirectionComponent, ConveyorComponent>(entity);

        if (conveyor.m_bIsCorner)
        {
            conveyor.m_Sequence = EntityId::Invalid();
            continue;
        }

        std::vector<Conveyor*> vConveyors;
        const Conveyor* pHeadConveyor = traceHeadConveyor(ecs, m_LookupGrid, position, direction, conveyor);
        traceTailConveyor(map, *pHeadConveyor, *pHeadConveyor, vConveyors);

        const auto end = vConveyors.end();
        std::vector<Conveyor*>::iterator chunk_begin;
        std::vector<Conveyor*>::iterator chunk_end;
        chunk_end = chunk_begin = vConveyors.begin();
        do
        {
            if (static_cast<uint32_t>(std::distance(chunk_end, end)) < Sequence::c_uiMaxSequenceLength)
            {
                chunk_end = end;
            }
            else
            {
                std::advance(chunk_end, Sequence::c_uiMaxSequenceLength);
            }

            std::vector<Conveyor*> vSequenceConveyors(chunk_begin, chunk_end);
            const auto pTailConveyor = vSequenceConveyors.front();

            vSequences.push_back(new Sequence(
                vSequenceConveyors[vSequenceConveyors.size() - 1],
                static_cast<uint8_t>(vSequenceConveyors.size()),
                pTailConveyor->m_pChannels[0].m_pSlots[0].m_VisualPosition,
                pTailConveyor->m_pChannels[1].m_pSlots[0].m_VisualPosition,
                pTailConveyor->m_pChannels[0].m_pSlots[1].m_VisualPosition - pTailConveyor->m_pChannels[0].m_pSlots[0].m_VisualPosition,
                vSequenceConveyors[vSequenceConveyors.size() - 1]->GetMoveTick()
            ));

            Sequence* sequence = vSequences.back();
            for(size_t i = 0; i < vSequenceConveyors.size(); ++i)
            {
                vSequenceConveyors[i]->SetSequence(sequence, static_cast<uint8_t>(i));
                alreadyProcessedConveyors.insert(vSequenceConveyors[i]);
            }

            chunk_begin = chunk_end;
        }
        while (std::distance(chunk_begin, end) > 0);
    }
}
