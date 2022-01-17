#include "TargetingUtility.h"
#include "Sequence.h"
#include "Profiler.h"
#include "Conveyor.h"
#include "EntityGrid.h"
#include "Direction.h"

const cpp_conv::Conveyor* cpp_conv::targeting_util::FindNextTailConveyor(const cpp_conv::WorldMap& map, const cpp_conv::Conveyor& rCurrentConveyor)
{
    static RelativeDirection directionPriority[] =
    {
        RelativeDirection::Backwards,
        RelativeDirection::Right,
        RelativeDirection::Left,
    };

    const cpp_conv::Conveyor* vPotentialNeighbours[4];
    vPotentialNeighbours[(int)RelativeDirection::Backwards] = map.GetEntity<cpp_conv::Conveyor>(cpp_conv::grid::GetBackwardsPosition(rCurrentConveyor), EntityKind::Conveyor);
    vPotentialNeighbours[(int)RelativeDirection::Right] = map.GetEntity<cpp_conv::Conveyor>(cpp_conv::grid::GetRightPosition(rCurrentConveyor), EntityKind::Conveyor);
    vPotentialNeighbours[(int)RelativeDirection::Left] = map.GetEntity<cpp_conv::Conveyor>(cpp_conv::grid::GetLeftPosition(rCurrentConveyor), EntityKind::Conveyor);

    const cpp_conv::Conveyor* pTargetConveyor = nullptr;
    for (auto direction : directionPriority)
    {
        const cpp_conv::Conveyor* pDirectionEntity = vPotentialNeighbours[(int)direction];
        if (pDirectionEntity == nullptr || pDirectionEntity->m_eEntityKind != EntityKind::Conveyor)
        {
            continue;
        }

        if (cpp_conv::grid::GetForwardPosition(*pDirectionEntity) == rCurrentConveyor.m_position)
        {
            pTargetConveyor = pDirectionEntity;
            break;
        }
    }

    return pTargetConveyor;
}

bool cpp_conv::targeting_util::IsCornerConveyor(const cpp_conv::WorldMap& map, const cpp_conv::Conveyor& rConveyor)
{
    PROFILE_FUNC();
    const cpp_conv::Conveyor* pBackConverter = cpp_conv::targeting_util::FindNextTailConveyor(map, rConveyor);
    if (pBackConverter == nullptr)
    {
        return false;
    }

    return pBackConverter->m_direction != rConveyor.m_direction;
}

bool cpp_conv::targeting_util::IsClockwiseCorner(const cpp_conv::WorldMap& map, const cpp_conv::Conveyor& rConveyor)
{
    PROFILE_FUNC();
    const cpp_conv::Conveyor* pBackConverter = cpp_conv::targeting_util::FindNextTailConveyor(map, rConveyor);
    if (pBackConverter == nullptr || pBackConverter->m_direction == rConveyor.m_direction)
    {
        return false;
    }

    Direction selfDirection = rConveyor.m_direction;
    Direction backDirection = pBackConverter->m_direction;
    while (selfDirection != Direction::Up)
    {
        selfDirection = cpp_conv::direction::Rotate90DegreeClockwise(selfDirection);
        backDirection = cpp_conv::direction::Rotate90DegreeClockwise(backDirection);
    }

    return backDirection == Direction::Right;
}

cpp_conv::Conveyor::Channel* cpp_conv::targeting_util::GetTargetChannel(const cpp_conv::WorldMap& map, const cpp_conv::Entity& sourceNode, cpp_conv::Conveyor& targetNode, int iSourceChannel)
{
    Direction eSourceDirection = sourceNode.GetDirection();    

    int mask = (int)eSourceDirection | (int)targetNode.m_direction;
    if (sourceNode.m_eEntityKind != EntityKind::Junction && (mask == 0b11 || (mask >> 2) == 0b11))
    {
        // The nodes are facing either other, no valid path.
        return nullptr;
    }

    if (IsCornerConveyor(map, targetNode))
    {
        if (&sourceNode == cpp_conv::targeting_util::FindNextTailConveyor(map, targetNode))
        {
            return &targetNode.m_pChannels[iSourceChannel];
        }

        return cpp_conv::targeting_util::IsClockwiseCorner(map, targetNode)
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

int cpp_conv::targeting_util::GetChannelTargetSlot(const cpp_conv::WorldMap& map, const cpp_conv::Entity& sourceNode, const cpp_conv::Conveyor& targetNode, int iSourceChannel)
{
    Direction eSourceDirection = sourceNode.GetDirection();
    if (eSourceDirection == targetNode.m_direction)
    {
        return 0;
    }

    int result = 0;

    if (IsCornerConveyor(map, targetNode) && &sourceNode == cpp_conv::targeting_util::FindNextTailConveyor(map, targetNode))
    {
        return 0;
    }

    switch (eSourceDirection)
    {
    case Direction::Up: result = targetNode.m_direction == Direction::Left ? iSourceChannel : cpp_conv::c_conveyorChannels - 1 - iSourceChannel; break;
    case Direction::Down: result = targetNode.m_direction == Direction::Left ? cpp_conv::c_conveyorChannels - 1 - iSourceChannel : iSourceChannel; break;
    case Direction::Left: result = targetNode.m_direction == Direction::Up ? iSourceChannel : cpp_conv::c_conveyorChannels - 1 - iSourceChannel; break;
    case Direction::Right: result = targetNode.m_direction == Direction::Up ? cpp_conv::c_conveyorChannels - 1 - iSourceChannel : iSourceChannel; break;
    }

    if (cpp_conv::targeting_util::IsCornerConveyor(map, targetNode))
    {
        result++;
    }

    return result;
}

Vector2F cpp_conv::targeting_util::GetRenderPosition(const cpp_conv::WorldMap& map, const cpp_conv::Conveyor& conveyor, ConveyorSlot slot, bool bAnimate, float fLerpFactor, Vector2F previousPosition)
{
    // This method translates the current direction in Right-facing space, determines the offsets, then rotates the offsets back to their original
    // direction-facing space.

    Direction eDirection = conveyor.GetDirection();
    int stepsRequired = 0;
    while (eDirection != Direction::Right)
    {
        eDirection = cpp_conv::direction::Rotate90DegreeClockwise(eDirection);
        stepsRequired++;
    }

    bool bIsCorner = cpp_conv::targeting_util::IsCornerConveyor(map, conveyor);
    bool bIsClockwiseConveyor = cpp_conv::targeting_util::IsClockwiseCorner(map, conveyor);

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
        position = { 1.0f + slot.m_Channel, 1.0f + slot.m_Lane };        
    }


    constexpr float c_fBlockSize = 4;
    Vector2F blockSize(c_fBlockSize, c_fBlockSize);
    Rotation backToOrigin = (Rotation)((4 - stepsRequired) % 4);
    position = position.Rotate(backToOrigin, blockSize);

    Vector2F offset = position * 0.5f * c_fBlockSize - Vector2F(1.0f, 1.0f) - (cpp_conv::renderer::c_gridScale / c_fBlockSize);

    Vector2F end = (Vector2F((float)conveyor.m_position.GetX(), (float)conveyor.m_position.GetY()) * blockSize) + offset;
    if (!bAnimate)
    {
        return end;
    }

    return previousPosition + ((end - previousPosition) * fLerpFactor);
}
