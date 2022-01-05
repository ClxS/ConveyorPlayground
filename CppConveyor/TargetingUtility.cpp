#include "TargetingUtility.h"
#include "Sequence.h"

cpp_conv::Conveyor::Channel* cpp_conv::targeting_util::GetTargetChannel(cpp_conv::Conveyor& sourceNode, cpp_conv::Conveyor& targetNode, int iSourceChannel)
{
    if (sourceNode.m_direction == targetNode.m_direction)
    {
        return &targetNode.m_pChannels[iSourceChannel];
    }

    int mask = (int)sourceNode.m_direction | (int)targetNode.m_direction;
    if (mask == 0b11 || (mask >> 2) == 0b11)
    {
        // The nodes are facing either other, no valid path.
        return nullptr;
    }

    switch (sourceNode.m_direction)
    {
    case Direction::Up: return targetNode.m_direction == Direction::Left ? &targetNode.m_pChannels[1] : &targetNode.m_pChannels[0];
    case Direction::Down: return targetNode.m_direction == Direction::Left ? &targetNode.m_pChannels[0] : &targetNode.m_pChannels[1];
    case Direction::Left: return targetNode.m_direction == Direction::Up ? &targetNode.m_pChannels[0] : &targetNode.m_pChannels[1];
    case Direction::Right: return targetNode.m_direction == Direction::Up ? &targetNode.m_pChannels[1] : &targetNode.m_pChannels[0];
    }

    return nullptr;
}

int cpp_conv::targeting_util::GetChannelTargetSlot(const grid::EntityGrid& grid, cpp_conv::Conveyor& sourceNode, cpp_conv::Conveyor& targetNode, int iSourceChannel)
{
    if (sourceNode.m_direction == targetNode.m_direction)
    {
        return 0;
    }

    int result = 0;

    switch (sourceNode.m_direction)
    {
    case Direction::Up: result = targetNode.m_direction == Direction::Left ? iSourceChannel : cpp_conv::c_conveyorChannels - 1 - iSourceChannel;
    case Direction::Down: result = targetNode.m_direction == Direction::Left ? cpp_conv::c_conveyorChannels - 1 - iSourceChannel : iSourceChannel;
    case Direction::Left: result = targetNode.m_direction == Direction::Up ? cpp_conv::c_conveyorChannels - 1 - iSourceChannel : iSourceChannel;
    case Direction::Right: result = targetNode.m_direction == Direction::Up ? iSourceChannel : cpp_conv::c_conveyorChannels - 1 - iSourceChannel;
    }

    if (cpp_conv::IsCornerConveyor(grid, targetNode))
    {
        result++;
    }

    return result;
}
