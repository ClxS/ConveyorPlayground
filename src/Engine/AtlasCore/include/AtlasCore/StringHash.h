#pragma once
#include <cstdint>
#include <string>

namespace atlas::core
{
    struct StringHashView
    {
        uint64_t m_StringHash;
        std::string_view m_String;

        explicit StringHashView(const std::string_view value)
            : m_StringHash{hashing::fnv1(value)}
            , m_String{value}
        {
        }
    };

    struct StringHash
    {
        uint64_t m_StringHash;
        std::string m_String;

        StringHash()
            : m_StringHash{0}
            , m_String{}
        {
        }

        explicit StringHash(std::string value)
            : m_StringHash{hashing::fnv1(value)}
            , m_String{std::move(value)}
        {
        }
    };

    constexpr bool operator==(const StringHash& a, const StringHash& b)
    {
        return a.m_StringHash == b.m_StringHash && a.m_String == b.m_String;
    }

    constexpr bool operator!=(const StringHash& a, const StringHash& b)
    {
        return a.m_StringHash != b.m_StringHash || a.m_String != b.m_String;
    }

    constexpr bool operator==(const StringHash& a, const StringHashView& b)
    {
        return a.m_StringHash == b.m_StringHash && a.m_String == b.m_String;
    }

    constexpr bool operator!=(const StringHash& a, const StringHashView& b)
    {
        return a.m_StringHash != b.m_StringHash || a.m_String != b.m_String;
    }
}
