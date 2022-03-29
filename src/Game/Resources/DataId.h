#pragma once

#include <cstdint>
#include <string>
#include "Serialization/DataField.h"

namespace cpp_conv
{
    static constexpr uint64_t idFromStringId(const std::string_view str)
    {
        uint64_t result = 0xcbf29ce484222325;
        for (const char c : str)
        {
            result *= 1099511628211;
            result ^= c;
        }

        return {result};
    }

#define DEFINE_UNIQUE_DATA_TYPE(NAME)\
    struct NAME##Id\
    {\
        uint64_t m_uiItemId;\
        static ItemId Empty() { return {0}; }\
        bool IsValid() const { return m_uiItemId != 0; }\
        bool IsEmpty() const { return m_uiItemId == 0; }\
        bool operator==(NAME##Id other) const\
        {\
            return m_uiItemId == other.m_uiItemId;\
        }\
        bool operator<(NAME##Id other) const\
        {\
            return m_uiItemId < other.m_uiItemId;\
        }\
        static NAME##Id FromStringId(const std::string_view str)\
        {\
            uint64_t uiId = idFromStringId(str);\
            return {uiId};\
        }\
    };\
    namespace NAME##Ids\
    {\
        constexpr NAME##Id None = { 0 };\
    }\
    template<>\
    struct TypedDataReader<NAME##Id>\
    {\
        static bool Read(const toml::Table* value, const char* szPropertyName, NAME##Id& pTargetVariable)\
        {\
            std::string strValue;\
            bool bResult = TypedDataReader<std::string>::Read(value, szPropertyName, strValue);\
            if (!bResult) return false;\
            pTargetVariable = NAME##Id::FromStringId(strValue);\
            return true;\
        }\
    }

    DEFINE_UNIQUE_DATA_TYPE(Item);

    DEFINE_UNIQUE_DATA_TYPE(Factory);

    DEFINE_UNIQUE_DATA_TYPE(Conveyor);

    DEFINE_UNIQUE_DATA_TYPE(Inserter);

    DEFINE_UNIQUE_DATA_TYPE(Recipe);
}
