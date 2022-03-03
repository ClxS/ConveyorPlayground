#pragma once

#include <chrono>

//#define ENABLE_PROFILE

namespace cpp_conv::profiler
{
    void registerTime(const char* szName, std::chrono::nanoseconds duration);
    void logAndReset(int factor);

    struct ScopeRaii
    {
        ScopeRaii(const char* szName)
        {
            m_szName = szName;
            m_start = std::chrono::high_resolution_clock::now();
        }

        ~ScopeRaii()
        {
            registerTime(m_szName, std::chrono::high_resolution_clock::now() - m_start);
        }

        const char* m_szName;
        std::chrono::steady_clock::time_point m_start;
    };
}

#if defined(ENABLE_PROFILE)
    #define STR_COMBINE_DIRECT(X,Y) X##Y
    #define STR_COMBINE2(X,Y) STR_COMBINE_DIRECT(X,Y)
    #define PROFILE(NAME, INSTRUCTION)\
        auto STR_COMBINE2(start, __LINE__) = std::chrono::high_resolution_clock::now();\
        INSTRUCTION;\
        auto STR_COMBINE2(end, __LINE__) = std::chrono::high_resolution_clock::now();\
        cpp_conv::profiler::registerTime(#NAME, STR_COMBINE2(end, __LINE__) - STR_COMBINE2(start, __LINE__));

    #define PROFILE_SCOPE(SCOPE)\
        cpp_conv::profiler::ScopeRaii STR_COMBINE2(scope, __LINE__)(#SCOPE);

    #define PROFILE_FUNC()\
        cpp_conv::profiler::ScopeRaii STR_COMBINE2(scope, __LINE__)(__FUNCTION__);
#else
#define PROFILE(NAME, INSTRUCTION) INSTRUCTION
#define PROFILE_SCOPE(SCOPE)
#define PROFILE_FUNC()
#endif
