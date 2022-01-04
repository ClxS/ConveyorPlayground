#pragma once

// Direction's explicit values are used by the simulation logic
// to optimize neighbour searching. Do not change them.
enum class Direction
{
    Up = (1 << 0),
    Down = (1 << 1),
    Left = (1 << 2),
    Right = (1 << 3),
};

enum class RelativeDirection
{
    Forward,
    Backwards,
    Right,
    Left
};

enum class EntityKind
{
    Conveyor,
    Producer,
    Junction,
    Underground
};