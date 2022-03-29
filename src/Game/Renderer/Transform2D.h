#pragma once
#include "Enums.h"
#include "Rotation.h"

namespace cpp_conv
{
    struct Transform2D
    {
        Transform2D()
            : m_x(0.0f)
              , m_y(0.0f)
              , m_bFillScreen(false)
              , m_rotation(Rotation::DegZero)
        {
        }

        Transform2D(float x, float y, bool bRepeating = false)
            : m_x(x)
              , m_y(y)
              , m_bFillScreen(bRepeating)
              , m_rotation(Rotation::DegZero)
        {
        }

        Transform2D(float x, float y, Rotation rotation, bool bRepeating = false)
            : m_x(x)
              , m_y(y)
              , m_bFillScreen(bRepeating)
              , m_rotation(rotation)
        {
        }

        float m_x;
        float m_y;
        bool m_bFillScreen;

        Rotation m_rotation;
    };

    inline Rotation rotationFromDirection(Direction direction)
    {
        // This engine uses right-based assets. Up/Down are inverted as we use a top major positioning system
        switch (direction)
        {
        default:
        case Direction::Right:
            return Rotation::DegZero;
        case Direction::Up:
            return Rotation::Deg90;
        case Direction::Left:
            return Rotation::Deg180;
        case Direction::Down:
            return Rotation::Deg270;
        }
    }
}
