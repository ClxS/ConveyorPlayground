#pragma once

#include <cstdint>

namespace atlas::scene
{
    struct ArchetypeIndex
    {
        int32_t m_ArchetypeIndex = -1;

        explicit ArchetypeIndex(const int32_t value)
            : m_ArchetypeIndex(value)
        {
        }

        static ArchetypeIndex Empty() { return ArchetypeIndex{0}; }

        bool operator==(const ArchetypeIndex other) const
        {
            return m_ArchetypeIndex == other.m_ArchetypeIndex;
        }

        [[nodiscard]] bool IsValid() const { return m_ArchetypeIndex >= 0; }
    };
}
