#include "Profiler.h"
#include "StringUtility.h"

#include <map>
#include <mutex>
#if _WIN32
#include <Windows.h>
#endif

namespace
{
    std::mutex& getStateMutex()
    {
        static std::mutex g_stateMutex;
        return g_stateMutex;
    }
}

std::map<const char*, std::chrono::nanoseconds> nameTimings;

void cpp_conv::profiler::registerTime(const char* szName, std::chrono::nanoseconds duration)
{
    std::lock_guard<std::mutex> lock(getStateMutex());
	nameTimings.try_emplace(szName, 0);
	nameTimings[szName] += duration;
}

void cpp_conv::profiler::logAndReset(int factor)
{
	std::lock_guard<std::mutex> lock(getStateMutex());
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