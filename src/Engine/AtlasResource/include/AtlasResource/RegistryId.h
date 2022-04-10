#pragma once
#include <cstdint>

namespace atlas::resource
{
    struct RegistryId
    {
        constexpr RegistryId(const int32_t value)
        : m_Value(value)
        {}

        static RegistryId Invalid() { return -1; }

        [[nodiscard]] bool IsValid() const { return m_Value >= 0; }

        bool operator<(const RegistryId& other) const { return m_Value < other.m_Value; }

        int32_t m_Value;
    };

    struct BundleRegistryId
    {
        RegistryId m_BundleId = -1;
        int32_t m_BundleIndex = -1;

        static BundleRegistryId Invalid() { return {RegistryId::Invalid(), -1}; }
        [[nodiscard]] bool IsValid() const { return m_BundleId.IsValid() && m_BundleIndex >= 0; }
    };
}
