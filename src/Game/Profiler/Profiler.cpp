#include "Profiler.h"

#include <mutex>
#include <unordered_map>

#include "AtlasCore/StringManipulation.h"
#if _WIN32
#include <Windows.h>
#endif

#define NEEDS_LOCKS 0

namespace
{
    std::mutex& getStateMutex()
    {
        static std::mutex g_stateMutex;
        return g_stateMutex;
    }
}

std::unordered_map<const char*, std::chrono::nanoseconds> nameTimings;

void cpp_conv::profiler::registerTime(const char* szName, std::chrono::nanoseconds duration)
{
#if NEEDS_LOCKS
    std::lock_guard<std::mutex> lock(getStateMutex());
#endif
    nameTimings.try_emplace(szName, 0);
    nameTimings[szName] += duration;
}

void cpp_conv::profiler::logAndReset(int factor)
{
#if NEEDS_LOCKS
    std::lock_guard<std::mutex> lock(getStateMutex());
#endif

    std::chrono::nanoseconds totalDuration = {};
    std::vector<std::pair<const char*, std::chrono::nanoseconds>> sortableTimings;

    for (auto& kvp : nameTimings)
    {
        totalDuration += kvp.second;
        sortableTimings.emplace_back(kvp.first, kvp.second);
    }

    std::ranges::sort(
        sortableTimings,
        [](const std::pair<const char*, std::chrono::nanoseconds>& a,
            const std::pair<const char*, std::chrono::nanoseconds>& b)
                      {
                          return a.second > b.second;
                      });

    for (auto& kvp : sortableTimings)
    {
        const auto percentage = (static_cast<double>(kvp.second.count()) / static_cast<double>(totalDuration.count())) *
            100;
        // Last than 1%? We don't care.
        if (percentage < 1.0)
        {
            continue;
        }

#if _WIN32
        OutputDebugStringA(
            std::format(
                "\n{}: {} ({}%)",
                kvp.first,
                atlas::core::string_manipulation::to_string_with_precision(
                    std::chrono::duration_cast<std::chrono::milliseconds>(kvp.second / factor)),
                atlas::core::string_manipulation::to_string_with_precision(percentage)).c_str());
#endif
    }

    for (auto& kvp : nameTimings)
    {
        nameTimings[kvp.first] = {};
    }
}
