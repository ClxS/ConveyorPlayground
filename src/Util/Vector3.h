#pragma once

#include <cstdint>

struct Vector3
{
    int32_t m_x;
    int32_t m_y;
    int32_t m_depth;

    Vector3 operator+(const Vector3& other) const
    {
        return { m_x + other.m_x, m_y + other.m_y, m_depth + other.m_depth };
    }

    Vector3& operator+=(const Vector3& other)
    {
        m_x += other.m_x;
        m_y += other.m_y;
        m_depth += other.m_depth;
        return *this;
    }

    Vector3 operator-(const Vector3& other) const
    {
        return { m_x - other.m_x, m_y - other.m_y, m_depth + other.m_depth };
    }

    bool operator==(const Vector3& other) const
    {
        return m_x == other.m_x && m_y == other.m_y && m_depth == other.m_depth;
    }
};
