#pragma once
#include <chrono>

namespace atlas::game::utility
{
    class FrameLimiter
    {
    public:
        FrameLimiter(int targetFrameRate);

        void Start();
        void Limit();
        void EndFrame();

    private:
        std::chrono::steady_clock::time_point m_startTime;
        std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<
                                    ptrdiff_t, std::ratio<1i64, 3000000000i64>>> m_nextFrameTime;
        std::chrono::microseconds m_targetFrameRate;
        std::chrono::duration<std::chrono::steady_clock::rep> m_logDuration;

        uint32_t m_uiFrameCounter;
    };
}
