#pragma once

#include "Conveyor.h"

namespace cpp_conv::targeting_util
{
    cpp_conv::Conveyor::Channel* GetTargetChannel(cpp_conv::Conveyor& sourceNode, cpp_conv::Conveyor& targetNode, int iSourceChannel);

    int GetChannelTargetSlot(const grid::EntityGrid& grid, cpp_conv::Conveyor& sourceNode, cpp_conv::Conveyor& targetNode, int iSourceChannel);
}