#pragma once

#include "Conveyor.h"
#include "EntityGrid.h"

namespace cpp_conv::targeting_util
{
    cpp_conv::Conveyor* FindNextTailConveyor(const grid::EntityGrid& grid, const cpp_conv::Conveyor& rCurrentConveyor);
    bool IsCornerConveyor(const grid::EntityGrid& grid, const Conveyor& rConveyor);
    bool IsClockwiseCorner(const grid::EntityGrid& grid, const Conveyor& rConveyor);

    cpp_conv::Conveyor::Channel* GetTargetChannel(const cpp_conv::grid::EntityGrid& grid, const cpp_conv::Entity& sourceNode, cpp_conv::Conveyor& targetNode, int iSourceChannel);

    int GetChannelTargetSlot(const grid::EntityGrid& grid, const cpp_conv::Entity& sourceNode, cpp_conv::Conveyor& targetNode, int iSourceChannel);
}
