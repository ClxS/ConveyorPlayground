#pragma once

#include <cstdint>

struct Position
{
    int32_t m_x;
    int32_t m_y;

    Position operator+(const Position& other) const
    {
        return { m_x + other.m_x, m_y + other.m_y };
    }

    Position operator-(const Position& other) const
    {
        return { m_x - other.m_x, m_y - other.m_y };
    }

    bool operator==(const Position& other) const
    {
        return m_x == other.m_x && m_y == other.m_y;
    }
};