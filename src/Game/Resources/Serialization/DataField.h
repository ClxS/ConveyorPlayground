#pragma once

#include "AssetRegistry.h"
#include "tomlcpp.hpp"
#include "Vector3.h"

namespace cpp_conv
{
    template <typename TReadType>
    struct TypedDataReader;

#define DEFINE_DATA_TYPE_HANDLER(TYPE) \
template<>\
struct TypedDataReader<TYPE>\
{\
static bool Read(const toml::Table* value, const char* szPropertyName, TYPE& pTargetVariable);\
};

    DEFINE_DATA_TYPE_HANDLER(bool);

    DEFINE_DATA_TYPE_HANDLER(int32_t);

    DEFINE_DATA_TYPE_HANDLER(int64_t);

    DEFINE_DATA_TYPE_HANDLER(uint32_t);

    DEFINE_DATA_TYPE_HANDLER(uint64_t);

    DEFINE_DATA_TYPE_HANDLER(float);

    DEFINE_DATA_TYPE_HANDLER(Vector3);

    DEFINE_DATA_TYPE_HANDLER(std::string);

    DEFINE_DATA_TYPE_HANDLER(resources::registry::RegistryId);

    struct DataFieldBase
    {
        DataFieldBase(const char* fieldName, bool bIsRequired)
            : m_FieldName{fieldName}
              , m_bIsRequired{bIsRequired}
              , m_bIsSet{false}
        {
        }

        virtual bool TryRead(toml::Table* rootTable) = 0;

        const char* m_FieldName;
        const bool m_bIsRequired;

        bool m_bIsSet;
    };

    template <size_t N>
    struct TemplateLiteralString
    {
        // ReSharper disable once CppNonExplicitConvertingConstructor
        constexpr TemplateLiteralString(const char (&str)[N])
        {
            std::copy_n(str, N, m_Value);
        }

        char m_Value[N];
    };

    template <typename TDataType, TemplateLiteralString DataEntryName, bool IsRequired = true>
    struct DataField : public DataFieldBase
    {
        DataField()
            : DataFieldBase(DataEntryName.m_Value, IsRequired)
        {
        }

        explicit DataField(TDataType initialValue)
            : DataFieldBase(DataEntryName.m_Value, IsRequired)
              , m_Value{initialValue}
        {
        }

        bool TryRead(toml::Table* rootTable) override
        {
            if (TypedDataReader<TDataType>::Read(rootTable, m_FieldName, m_Value))
            {
                m_bIsSet = true;
                return true;
            }

            m_bIsSet = false;
            return false;
        }


        TDataType m_Value;
    };
}
