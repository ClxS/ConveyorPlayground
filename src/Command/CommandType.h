#pragma once

namespace cpp_conv::commands
{
    enum class CommandType
    {
        MoveLeft,
        MoveUp,
        MoveRight,
        MoveDown,

        PlaceConveyorUp,
        PlaceConveyorDown,
        PlaceConveyorLeft,
        PlaceConveyorRight,

        DecrementZoom,
        IncrementZoom,
    };
}
