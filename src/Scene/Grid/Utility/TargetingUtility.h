#pragma once

#include "Conveyor.h"
#include "EntityGrid.h"
#include "WorldMap.h"

namespace cpp_conv::targeting_util
{
    const cpp_conv::Conveyor* FindNextTailConveyor(const cpp_conv::WorldMap& map, const cpp_conv::Conveyor& rCurrentConveyor);
    bool IsCornerConveyor(const cpp_conv::WorldMap& map, const Conveyor& rConveyor);
    bool IsClockwiseCorner(const cpp_conv::WorldMap& map, const Conveyor& rConveyor);

    cpp_conv::Conveyor::Channel* GetTargetChannel(const cpp_conv::WorldMap& map, const cpp_conv::Entity& sourceNode, cpp_conv::Conveyor& targetNode, int iSourceChannel);

    int GetChannelTargetSlot(const cpp_conv::WorldMap& map, const cpp_conv::Entity& sourceNode, cpp_conv::Conveyor& targetNode, int iSourceChannel);
}
