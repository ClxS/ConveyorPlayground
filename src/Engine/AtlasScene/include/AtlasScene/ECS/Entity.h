#pragma once

#include <cstdint>

namespace atlas::scene
{
    struct EntityId
    {
        static constexpr int c_invalidValue = -1;

        EntityId()
        {
        }

        explicit EntityId(const int32_t value)
            : m_Value(value)
        {
        }

        static EntityId Invalid() { return EntityId{c_invalidValue}; }

        bool IsInvalid() const { return m_Value == c_invalidValue; }

        bool operator==(const EntityId other) const
        {
            return m_Value == other.m_Value;
        }

        bool operator<(const EntityId other) const
        {
            return m_Value < other.m_Value;
        }

        [[nodiscard]] bool IsValid() const { return m_Value >= 0; }

        int32_t m_Value = -1;
    };
}
