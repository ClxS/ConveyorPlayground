#pragma once

#include <cstdint>

namespace cpp_conv
{
    struct ItemId
    {
        uint64_t m_uiItemId;

        bool IsValid() const { return m_uiItemId != 0; }
        bool IsEmpty() const { return m_uiItemId == 0; }

        bool operator==(ItemId other) const
        {
            return m_uiItemId == other.m_uiItemId;
        }
    };

    namespace Item
    {
        constexpr ItemId None = { 0 };
    }
}


