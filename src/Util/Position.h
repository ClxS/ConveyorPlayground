#pragma once

#include <cstdint>

struct Position
{
    int32_t m_x;
    int32_t m_y;
    int32_t m_depth;

    Position operator+(const Position& other) const
    {
        return { m_x + other.m_x, m_y + other.m_y, m_depth + other.m_depth };
    }

    Position& operator+=(const Position& other)
    {
        m_x += other.m_x;
        m_y += other.m_y;
        m_depth += other.m_depth;
        return *this;
    }

    Position operator-(const Position& other) const
    {
        return { m_x - other.m_x, m_y - other.m_y, m_depth + other.m_depth };
    }

    bool operator==(const Position& other) const
    {
        return m_x == other.m_x && m_y == other.m_y && m_depth == other.m_depth;
    }
};
