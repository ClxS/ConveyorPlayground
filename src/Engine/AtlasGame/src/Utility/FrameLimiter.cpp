#include "AtlasGamePCH.h"
#include "Utility/FrameLimiter.h"

#include <thread>
#include <Windows.h>

#include "AtlasCore/StringManipulation.h"

atlas::game::utility::FrameLimiter::FrameLimiter(const int targetFrameRate)
    : m_startTime({})
      , m_targetFrameRate(1000000 / targetFrameRate)
      , m_logDuration(std::chrono::seconds(5))
      , m_uiFrameCounter(0)
{
}

void atlas::game::utility::FrameLimiter::Start()
{
    m_startTime = std::chrono::high_resolution_clock::now();
    m_nextFrameTime = m_startTime + m_targetFrameRate;
}

void atlas::game::utility::FrameLimiter::Limit()
{
    std::this_thread::sleep_until(m_nextFrameTime);
    m_nextFrameTime += m_targetFrameRate;
}

void atlas::game::utility::FrameLimiter::EndFrame()
{
    m_uiFrameCounter++;
    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - m_startTime) >=
        m_logDuration)
    {
#if _WIN32
        OutputDebugStringA(std::format("\n\nFPS: {}",
                                       core::string_manipulation::to_string_with_precision(
                                           m_uiFrameCounter / static_cast<double>(m_logDuration.count()), 0)).c_str());
#endif

        m_startTime = std::chrono::high_resolution_clock::now();
        m_uiFrameCounter = 0;
    }
}
