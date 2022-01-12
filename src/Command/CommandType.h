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

        DecrementZoom,
        IncrementZoom,

        PlaceSelection,
        DeleteSelection,

        SelectItem1,
        SelectItem2,
        SelectItem3,
        SelectItem4,
        SelectItem5,
        SelectItem6,
        SelectItem7,
        SelectItem8,
        SelectItem9,
        RotateSelection,
        ToggleModifier,
    };
}
