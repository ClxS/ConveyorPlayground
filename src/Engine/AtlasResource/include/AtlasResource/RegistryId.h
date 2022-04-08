#pragma once
#include <cstdint>

namespace atlas::resource
{
    struct RegistryId
    {
        constexpr RegistryId(const uint32_t value)
        : m_Value(value)
        {}

        static RegistryId Invalid() { return 0xFFFFFFFF; }

        bool operator<(const RegistryId& other) const { return m_Value < other.m_Value; }

        uint32_t m_Value;
    };
}
