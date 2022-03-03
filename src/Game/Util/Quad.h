#pragma once
#include <cstdint>

struct Quad
{
    int32_t m_x;
    int32_t m_y;
    int32_t m_uiWidth;
    int32_t m_uiHeight;

    [[nodiscard]] int32_t GetLeft() const { return m_x; }
    [[nodiscard]] int32_t GetTop() const { return m_y; }
    [[nodiscard]] int32_t GetRight() const { return m_x + m_uiWidth; }
    [[nodiscard]] int32_t GetBottom() const { return m_y + m_uiHeight; }
};
