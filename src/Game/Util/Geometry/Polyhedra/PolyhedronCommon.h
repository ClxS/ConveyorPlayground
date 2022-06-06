#pragma once

#include <Eigen/Core>

namespace cpp_conv::util::geometry::polyhedron
{
    template<uint32_t TIndexCount>
    struct Polygon
    {
        Eigen::Matrix<int, TIndexCount, 1, 0> m_Indices{};
    };

    struct Point
    {
        float m_X;
        float m_Y;
        float m_Z;

        float m_U;
        float m_V;

        Point operator+(const Point& other) const
        {
            return { m_X + other.m_X, m_Y + other.m_Y, m_Z + other.m_Z, m_U, m_V };
        }

        Point operator-(const Point& other) const
        {
            return { m_X - other.m_X, m_Y - other.m_Y, m_Z - other.m_Z, m_U, m_V };
        }

        Point& operator+=(const Point& other)
        {
            *this = *this + other;
            return *this;
        }

        Point& operator-=(const Point& other)
        {
            *this = *this - other;
            return *this;
        }
    };
}
