#pragma once
#include <sstream>
#include <string>

namespace atlas::core::string_manipulation
{
    template <typename T>
    std::string to_string_with_precision(const T a_value, const int n = 2)
    {
        std::ostringstream out;
        out.precision(n);
        out << std::fixed << a_value;
        return out.str();
    }
}
