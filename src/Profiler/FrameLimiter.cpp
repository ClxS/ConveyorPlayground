#include "FrameLimiter.h"

#include "Profiler.h"
#include "StringUtility.h"

#include <format>
#include <thread>
#if _WIN32
#include <Windows.h>
#endif

cpp_conv::FrameLimiter::FrameLimiter(const int targetFrameRate)
    : m_startTime({})
    , m_targetFrameRate(1000000 / targetFrameRate)
    , m_logDuration(std::chrono::seconds(5))
    , m_uiFrameCounter(0)
{
}

void cpp_conv::FrameLimiter::Start()
{
    m_startTime = std::chrono::high_resolution_clock::now();
    m_nextFrameTime = m_startTime + m_targetFrameRate;
}

void cpp_conv::FrameLimiter::Limit()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    std::this_thread::sleep_until(m_nextFrameTime);
    m_nextFrameTime += m_targetFrameRate;
}

void cpp_conv::FrameLimiter::EndFrame()
{
    m_uiFrameCounter++;
    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - m_startTime) >= m_logDuration)
    {
#if _WIN32
        OutputDebugStringA(std::format("\n\nFPS: {}", string_util::to_string_with_precision(m_uiFrameCounter / static_cast<double>(m_logDuration.count()), 0)).c_str());
#endif
        profiler::logAndReset(static_cast<int>(m_uiFrameCounter));

        m_startTime = std::chrono::high_resolution_clock::now();
        m_uiFrameCounter = 0;
    }
}
