#pragma once

#include <cstdint>
#include <string_view>

namespace atlas::core::hashing
{
    static constexpr uint64_t fnv1(const std::string_view str)
    {
        uint64_t result = 0xcbf29ce484222325;
        for (const char c : str)
        {
            result *= 1099511628211;
            result ^= c;
        }

        return result;
    }
}
