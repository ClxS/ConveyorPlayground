#include "ConveyorHelper.h"

#include "DirectionComponent.h"
#include "EntityGrid.h"
#include "PositionComponent.h"
#include "WorldEntityInformationComponent.h"

#include <AtlasScene/ECS/Components/EcsManager.h>

#include "EntityLookupGrid.h"
#include "FactoryComponent.h"

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
    vPositions[static_cast<int>(RelativeDirection::Backwards)] = grid::getBackwardsPosition(position, direction);
    vPositions[static_cast<int>(RelativeDirection::Right)] = grid::getRightPosition(position, direction);
    vPositions[static_cast<int>(RelativeDirection::Left)] = grid::getLeftPosition(position, direction);

    EntityId pTargetConveyor = EntityId::Invalid();
    for (auto d : directionPriority)
    {
        EntityId directionEntity = grid.GetEntity(vPositions[static_cast<int>(d)]);
        if (directionEntity.IsInvalid() || !ecs.DoesEntityHaveComponents<
            PositionComponent, DirectionComponent, WorldEntityInformationComponent>(directionEntity))
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
                auto [targetPosition, targetDirection] = ecs.GetComponents<PositionComponent, DirectionComponent>(
                    directionEntity);
                if (grid::getForwardPosition(targetPosition.m_Position, targetDirection.m_Direction) == position)
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
                    PositionComponent, DirectionComponent, FactoryComponent>(directionEntity);
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
