#pragma once
#include "bgfx/bgfx.h"
#include "bx/bx.h"

namespace atlas::render
{
    class FrameBuffer
    {
    public:
        ~FrameBuffer();

        void Initialise(const uint32_t width, const uint32_t height);

        void EnsureSize(uint32_t width, uint32_t height);

        [[nodiscard]] bgfx::FrameBufferHandle GetHandle() const { return m_Handle; }

    private:
        uint32_t m_Width{};
        uint32_t m_Height{};

        bgfx::FrameBufferHandle m_Handle{BGFX_INVALID_HANDLE};
    };
}
