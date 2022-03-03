#pragma once

#include "Conveyor.h"
#include "EntityGrid.h"

#include <cstdint>

namespace cpp_conv::targeting_util
{
    struct ConveyorSlot
    {
        int32_t m_Lane;
        int32_t m_Channel;
    };

    Entity* findNextTailConveyor(const WorldMap& map, const Conveyor& rCurrentConveyor, RelativeDirection& outDirection);
    bool isCornerConveyor(const WorldMap& map, const Conveyor& rConveyor);
    bool isClockwiseCorner(const WorldMap& map, const Conveyor& rConveyor);

    Conveyor::Channel* getTargetChannel(const WorldMap& map, const Entity& sourceNode, Conveyor& targetNode, int iSourceChannel);

    int getChannelTargetSlot(const WorldMap& map, const Entity& sourceNode, const Conveyor& targetNode, int iSourceChannel);

    Vector2F getTilePosition(
        const WorldMap& map,
        const Entity& conveyor,
        ConveyorSlot slot);

    Vector2F getRenderPosition(
        const WorldMap& map,
        const Conveyor& conveyor,
        ConveyorSlot slot,
        bool bAnimate = false,
        float fLerpFactor = 1.0f,
        Vector2F previousPosition = {});

    Vector2F getRenderPosition(
        const WorldMap& map,
        const Conveyor& conveyor,
        Vector2F renderPosition,
        float fLerpFactor,
        Vector2F previousPosition);
}
