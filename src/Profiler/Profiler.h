#pragma once

#include <chrono>

namespace cpp_conv::profiler
{
    void registerTime(const char* szName, std::chrono::nanoseconds duration);
    void logAndReset(int factor);
}

#define STR_COMBINE_DIRECT(X,Y) X##Y
#define STR_COMBINE2(X,Y) STR_COMBINE_DIRECT(X,Y)
#define PROFILE(NAME, INSTRUCTION)\
    auto STR_COMBINE2(start, __LINE__) = std::chrono::high_resolution_clock::now();\
    INSTRUCTION;\
    auto STR_COMBINE2(end, __LINE__) = std::chrono::high_resolution_clock::now();\
    cpp_conv::profiler::registerTime(#NAME, STR_COMBINE2(end, __LINE__) - STR_COMBINE2(start, __LINE__));