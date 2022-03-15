#pragma once

#include <cstdint>

namespace atlas::scene
{
    struct EntityId
    {
        int32_t m_Value = -1;

        explicit EntityId(const int32_t value)
            : m_Value(value)
        {
        }

        static EntityId Invalid() { return EntityId{ -1 }; }

        bool operator==(const EntityId other) const
        {
            return m_Value == other.m_Value;
        }

        [[nodiscard]] bool IsValid() const { return m_Value >= 0; }
    };
}
