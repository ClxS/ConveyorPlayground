#pragma once
#include "bgfx/bgfx.h"
#include "bx/bx.h"

namespace atlas::render
{
    class FrameBuffer
    {
    public:
        FrameBuffer();
        ~FrameBuffer();

        void Initialise(uint32_t width, const uint32_t height, bool includeDepth = true);

        void EnsureSize(uint32_t width, uint32_t height);

        [[nodiscard]] bgfx::FrameBufferHandle GetHandle() const { return m_Handle; }

    private:
        uint32_t m_Width{};
        uint32_t m_Height{};

        bgfx::FrameBufferHandle m_Handle{BGFX_INVALID_HANDLE};
    };
}
