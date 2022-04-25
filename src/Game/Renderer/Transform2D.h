#pragma once
#include "Enums.h"
#include "Rotation.h"
#include "AtlasCore/MathsHelpers.h"

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

    inline atlas::maths_helpers::Angle rotationRadiansFromDirection(const Direction direction)
    {
        constexpr float c_pi = 3.14159265358979323846f;  /* pi */
        // This engine uses right-based assets. Up/Down are inverted as we use a top major positioning system
        switch (direction)
        {
        default:
        case Direction::Right:
            return 0.0_degrees;
        case Direction::Down:
            return 90.0_degrees;
        case Direction::Left:
            return 180.0_degrees;
        case Direction::Up:
            return 270.0_degrees;
        }
    }
}
