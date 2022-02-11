#include "TargetingUtility.h"
#include "Conveyor.h"
#include "Direction.h"
#include "EntityGrid.h"
#include "Factory.h"
#include "Profiler.h"
#include "WorldMap.h"

cpp_conv::Entity* cpp_conv::targeting_util::findNextTailConveyor(const WorldMap& map, const Conveyor& rCurrentConveyor, RelativeDirection& outDirection)
{
    static RelativeDirection directionPriority[] =
    {
        RelativeDirection::Backwards,
        RelativeDirection::Right,
        RelativeDirection::Left,
    };

    Vector3 vPositions[4];
    vPositions[static_cast<int>(RelativeDirection::Backwards)] = grid::getBackwardsPosition(rCurrentConveyor);
    vPositions[static_cast<int>(RelativeDirection::Right)] = grid::getRightPosition(rCurrentConveyor);
    vPositions[static_cast<int>(RelativeDirection::Left)] = grid::getLeftPosition(rCurrentConveyor);

    Entity* vPotentialNeighbours[4];
    vPotentialNeighbours[static_cast<int>(RelativeDirection::Backwards)] = map.GetEntity(vPositions[static_cast<int>(RelativeDirection::Backwards)]);
    vPotentialNeighbours[static_cast<int>(RelativeDirection::Right)] = map.GetEntity(vPositions[static_cast<int>(RelativeDirection::Right)]);
    vPotentialNeighbours[static_cast<int>(RelativeDirection::Left)] = map.GetEntity(vPositions[static_cast<int>(RelativeDirection::Left)]);

    Entity* pTargetConveyor = nullptr;
    for (auto direction : directionPriority)
    {
        Entity* pDirectionEntity = vPotentialNeighbours[static_cast<int>(direction)];
        if (pDirectionEntity == nullptr)
        {
            continue;
        }

        switch (pDirectionEntity->m_eEntityKind)
        {
            case EntityKind::Stairs:
            case EntityKind::Underground:
            case EntityKind::Conveyor:
                if (grid::getForwardPosition(*pDirectionEntity) == rCurrentConveyor.m_position)
                {
                    pTargetConveyor = pDirectionEntity;
                }
                break;
            case EntityKind::Junction:
                pTargetConveyor = pDirectionEntity;
                break;
            case EntityKind::Producer:
            {
                const Factory* pFactory = reinterpret_cast<Factory*>(pDirectionEntity);
                if (pFactory->HasOutputPipe() && vPositions[static_cast<int>(direction)] == pFactory->m_position + pFactory->GetOutputPipe())
                {
                    pTargetConveyor = pDirectionEntity;
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

        if (pTargetConveyor)
        {
            outDirection = direction;
            break;
        }
    }

    return pTargetConveyor;
}

bool cpp_conv::targeting_util::isCornerConveyor(const WorldMap& map, const Conveyor& rConveyor)
{
    PROFILE_FUNC();
    RelativeDirection direction;
    const Entity* pBackConverter = findNextTailConveyor(map, rConveyor, direction);
    if (pBackConverter == nullptr || direction == RelativeDirection::Backwards || direction == RelativeDirection::Forward)
    {
        return false;
    }

    return pBackConverter->GetDirection() != rConveyor.GetDirection() || pBackConverter->m_eEntityKind == EntityKind::Junction;
}

bool cpp_conv::targeting_util::isClockwiseCorner(const WorldMap& map, const Conveyor& rConveyor)
{
    PROFILE_FUNC();
    RelativeDirection direction;
    const Entity* pBackConverter = findNextTailConveyor(map, rConveyor, direction);
    if (pBackConverter == nullptr || direction == RelativeDirection::Backwards || direction == RelativeDirection::Forward)
    {
        return false;
    }

    return (direction == RelativeDirection::Left);
}

cpp_conv::Conveyor::Channel* cpp_conv::targeting_util::getTargetChannel(const WorldMap& map, const Entity& sourceNode,
                                                                        Conveyor& targetNode, int iSourceChannel)
{
    Direction eSourceDirection = sourceNode.GetDirection();

    const int mask = static_cast<int>(eSourceDirection) | static_cast<int>(targetNode.m_direction);
    if (sourceNode.m_eEntityKind != EntityKind::Junction && (mask == 0b11 || (mask >> 2) == 0b11))
    {
        // The nodes are facing either other, no valid path.
        return nullptr;
    }

    if (isCornerConveyor(map, targetNode))
    {
        RelativeDirection direction;
        if (&sourceNode == findNextTailConveyor(map, targetNode, direction))
        {
            return &targetNode.m_pChannels[iSourceChannel];
        }

        return isClockwiseCorner(map, targetNode)
                   ? &targetNode.m_pChannels[0]
                   : &targetNode.m_pChannels[1];
    }

    if (eSourceDirection == targetNode.m_direction)
    {
        return &targetNode.m_pChannels[iSourceChannel];
    }

    switch (eSourceDirection)
    {
    case Direction::Up: return targetNode.m_direction == Direction::Left ? &targetNode.m_pChannels[1] : &targetNode.m_pChannels[0];
    case Direction::Down: return targetNode.m_direction == Direction::Left ? &targetNode.m_pChannels[0] : &targetNode.m_pChannels[1];
    case Direction::Left: return targetNode.m_direction == Direction::Up ? &targetNode.m_pChannels[0] : &targetNode.m_pChannels[1];
    case Direction::Right: return targetNode.m_direction == Direction::Up ? &targetNode.m_pChannels[1] : &targetNode.m_pChannels[0];
    }

    return nullptr;
}

int cpp_conv::targeting_util::getChannelTargetSlot(const WorldMap& map, const Entity& sourceNode, const Conveyor& targetNode, int iSourceChannel)
{
    const Direction eSourceDirection = sourceNode.GetDirection();
    if (eSourceDirection == targetNode.m_direction)
    {
        return 0;
    }

    int result = 0;

    RelativeDirection direction;
    if (isCornerConveyor(map, targetNode) && &sourceNode == findNextTailConveyor(map, targetNode, direction))
    {
        return 0;
    }

    switch (eSourceDirection)
    {
    case Direction::Up: result = targetNode.m_direction == Direction::Left ? iSourceChannel : c_conveyorChannels - 1 - iSourceChannel; break;
    case Direction::Down: result = targetNode.m_direction == Direction::Left ? c_conveyorChannels - 1 - iSourceChannel : iSourceChannel; break;
    case Direction::Left: result = targetNode.m_direction == Direction::Up ? c_conveyorChannels - 1 - iSourceChannel : iSourceChannel; break;
    case Direction::Right: result = targetNode.m_direction == Direction::Up ? iSourceChannel : c_conveyorChannels - 1 - iSourceChannel; break;
    }

    if (isCornerConveyor(map, targetNode))
    {
        result++;
    }

    return result;
}

Vector2F cpp_conv::targeting_util::getRenderPosition(const WorldMap& map, const Conveyor& conveyor, ConveyorSlot slot, bool bAnimate, float fLerpFactor, Vector2F previousPosition)
{
    PROFILE_FUNC();
    // This method translates the current direction in Right-facing space, determines the offsets, then rotates the offsets back to their original
    // direction-facing space.

    Direction eDirection = conveyor.GetDirection();
    int stepsRequired = 0;
    while (eDirection != Direction::Right)
    {
        eDirection = direction::rotate90DegreeClockwise(eDirection);
        stepsRequired++;
    }

    const bool bIsCorner = isCornerConveyor(map, conveyor);
    const bool bIsClockwiseConveyor = isClockwiseCorner(map, conveyor);

    Vector2F position;
    if (bIsCorner)
    {
        if (bIsClockwiseConveyor)
        {
            if (slot.m_Lane == 0)
            {
                switch (slot.m_Channel)
                {
                    case 0: position = { 1.0f, 2.0f }; break;
                    case 1: position = { 1.2f, 1.2f }; break;
                    case 2: position = { 2.0f, 1.0f }; break;
                }
            }
            else
            {
                position = { 2.0f, 2.0f };
            }
        }
        else
        {
            if (slot.m_Lane == 0)
            {
                position = { 2.0f, 1.0f };
            }
            else
            {
                switch (slot.m_Channel)
                {
                    case 0: position = { 1.0f, 1.0f }; break;
                    case 1: position = { 1.2f, 1.8f }; break;
                    case 2: position = { 2.0f, 2.0f }; break;
                }
            }
        }
    }
    else
    {
        position = { 1.0f + static_cast<float>(slot.m_Channel), 1.0f + static_cast<float>(slot.m_Lane) };
    }

    constexpr float c_fBlockSize = 4;
    const Vector2F blockSize(c_fBlockSize, c_fBlockSize);
    const auto backToOrigin = static_cast<Rotation>((4 - stepsRequired) % 4);
    position = position.Rotate(backToOrigin, blockSize);

    const Vector2F offset = position * 0.5f * c_fBlockSize - Vector2F(1.0f, 1.0f) - (renderer::c_gridScale / c_fBlockSize);

    const Vector2F end = (Vector2F(static_cast<float>(conveyor.m_position.GetX()), static_cast<float>(conveyor.m_position.GetY())) * blockSize) + offset;
    if (!bAnimate)
    {
        return end;
    }

    return previousPosition + ((end - previousPosition) * fLerpFactor);
}

Vector2F cpp_conv::targeting_util::getRenderPosition(
    const WorldMap& map,
    const Conveyor& conveyor,
    Vector2F renderPosition,
    float fLerpFactor,
    Vector2F previousPosition)
{
    PROFILE_FUNC();
    return previousPosition + ((renderPosition - previousPosition) * fLerpFactor);
}
