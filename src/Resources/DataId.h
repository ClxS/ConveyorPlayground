#pragma once

#include <cstdint>

namespace cpp_conv
{
    #define DEFINE_UNIQUE_DATA_TYPE(NAME)\
    struct NAME##Id\
    {\
        uint64_t m_uiItemId;\
        bool IsValid() const { return m_uiItemId != 0; }\
        bool IsEmpty() const { return m_uiItemId == 0; }\
        bool operator==(NAME##Id other) const\
        {\
            return m_uiItemId == other.m_uiItemId;\
        }\
    };\
    namespace NAME\
    {\
        constexpr NAME##Id None = { 0 };\
    }

    DEFINE_UNIQUE_DATA_TYPE(Item);
    DEFINE_UNIQUE_DATA_TYPE(Producer);
}


