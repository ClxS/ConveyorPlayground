#pragma once

namespace cpp_conv::commands
{
    enum class CommandType
    {
        MoveLeft,
        MoveUp,
        MoveRight,
        MoveDown,

        MoveFloorUp,
        MoveFloorDown,

        PlaceConveyorUp,
        PlaceConveyorDown,
        PlaceConveyorLeft,
        PlaceConveyorRight,

        PlaceStairsUp,
        PlaceStairsDown,

        DecrementZoom,
        IncrementZoom,
    };
}
