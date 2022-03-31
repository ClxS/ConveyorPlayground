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
              , m_rotation(0.0f)
        {
        }

        Transform2D(float x, float y, bool bRepeating = false)
            : m_x(x)
              , m_y(y)
              , m_bFillScreen(bRepeating)
              , m_rotation(0.0f)
        {
        }

        Transform2D(float x, float y, float rotation, bool bRepeating = false)
            : m_x(x)
              , m_y(y)
              , m_bFillScreen(bRepeating)
              , m_rotation(rotation)
        {
        }

        float m_x;
        float m_y;
        bool m_bFillScreen;

        float m_rotation;
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

    inline float rotationRadiansFromDirection(Direction direction)
    {
        constexpr float c_pi = 3.14159265358979323846f;  /* pi */
        // This engine uses right-based assets. Up/Down are inverted as we use a top major positioning system
        switch (direction)
        {
        default:
        case Direction::Right:
            return 0.0f;
        case Direction::Up:
            return 90.0f * (c_pi / 180.0f);
        case Direction::Left:
            return 180.0f * (c_pi / 180.0f);
        case Direction::Down:
            return 270.0f * (c_pi / 180.0f);
        }
    }
}
