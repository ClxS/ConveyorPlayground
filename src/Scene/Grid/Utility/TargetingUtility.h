#pragma once

#include "Conveyor.h"
#include "EntityGrid.h"
#include "WorldMap.h"

#include <cstdint>

namespace cpp_conv::targeting_util
{
    struct ConveyorSlot
    {
        int32_t m_Lane;
        int32_t m_Channel;
    };

    cpp_conv::Entity* FindNextTailConveyor(const cpp_conv::WorldMap& map, const cpp_conv::Conveyor& rCurrentConveyor);
    bool IsCornerConveyor(const cpp_conv::WorldMap& map, const Conveyor& rConveyor);
    bool IsClockwiseCorner(const cpp_conv::WorldMap& map, const Conveyor& rConveyor);

    cpp_conv::Conveyor::Channel* GetTargetChannel(const cpp_conv::WorldMap& map, const cpp_conv::Entity& sourceNode, cpp_conv::Conveyor& targetNode, int iSourceChannel);

    int GetChannelTargetSlot(const cpp_conv::WorldMap& map, const cpp_conv::Entity& sourceNode, const cpp_conv::Conveyor& targetNode, int iSourceChannel);

    Vector2F GetRenderPosition(
        const cpp_conv::WorldMap& map,
        const cpp_conv::Conveyor& conveyor,
        ConveyorSlot slot,
        bool bAnimate = false,
        float fLerpFactor = 1.0f,
        Vector2F previousPosition = {});

    Vector2F GetRenderPosition(
        const cpp_conv::WorldMap& map,
        const cpp_conv::Conveyor& conveyor,
        Vector2F renderPosition,
        float fLerpFactor,
        Vector2F previousPosition);
}
