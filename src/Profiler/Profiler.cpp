#include "Profiler.h"
#include <map>

#include "StringUtility.h"

#if _WIN32
#include <Windows.h>
#endif

std::map<const char*, std::chrono::nanoseconds> nameTimings;

void cpp_conv::profiler::registerTime(const char* szName, std::chrono::nanoseconds duration)
{
	nameTimings.try_emplace(szName, 0);
	nameTimings[szName] += duration;
}

void cpp_conv::profiler::logAndReset(int factor)
{
    std::chrono::nanoseconds totalDuration = {};
    for (auto& kvp : nameTimings)
    {
        totalDuration += kvp.second;
    }

    for (auto& kvp : nameTimings)
    {
#if _WIN32
        OutputDebugStringA(
            std::format(
                "\n{}: {} ({}%)",
                kvp.first,
                cpp_conv::string_util::to_string_with_precision(std::chrono::duration_cast<std::chrono::milliseconds>(kvp.second / factor)),
                cpp_conv::string_util::to_string_with_precision(((double)kvp.second.count() / (double)totalDuration.count()) * 100)).c_str());
#endif
    }

    for (auto& kvp : nameTimings)
    {
        nameTimings[kvp.first] = {};
    }
}
