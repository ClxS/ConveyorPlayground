#include "AtlasRenderPCH.h"
#include "Types/FrameBuffer.h"

atlas::render::FrameBuffer::FrameBuffer()
{

}

atlas::render::FrameBuffer::~FrameBuffer()
{
    bgfx::destroy(m_Handle);
}

void atlas::render::FrameBuffer::Initialise(const uint32_t width, const uint32_t height, bool includeDepth)
{
    m_Width = width;
    m_Height = height;

    if (isValid(m_Handle))
    {
        destroy(m_Handle);
    }

    const auto colour = bgfx::createTexture2D(
        static_cast<uint16_t>(m_Width),
        static_cast<uint16_t>(m_Height),
        false,
        1,
        bgfx::TextureFormat::RGBA32F,
        BGFX_TEXTURE_RT);

    if (includeDepth)
    {
        const bgfx::TextureFormat::Enum depthFormat =
              bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D16,   BGFX_TEXTURE_RT_WRITE_ONLY) ? bgfx::TextureFormat::D16
            : bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT_WRITE_ONLY) ? bgfx::TextureFormat::D24S8
            : bgfx::TextureFormat::D32
            ;

        const auto depth = bgfx::createTexture2D(
            static_cast<uint16_t>(m_Width),
            static_cast<uint16_t>(m_Height),
            false,
            1,
            depthFormat,
            BGFX_TEXTURE_RT_WRITE_ONLY);

        const bgfx::TextureHandle handles[] =
        {
            colour,
            depth
        };

        m_Handle = bgfx::createFrameBuffer(BX_COUNTOF(handles), handles, true);
    }
    else
    {
        const bgfx::TextureHandle handles[] =
        {
            colour,
        };

        m_Handle = bgfx::createFrameBuffer(BX_COUNTOF(handles), handles, true);
    }
}

void atlas::render::FrameBuffer::EnsureSize(const uint32_t width, const uint32_t height)
{
    if (width != m_Width || height != m_Height)
    {
        Initialise(width, height);
    }
}
