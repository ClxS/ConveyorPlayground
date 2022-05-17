#include "PostProcessSystem.h"

#include "AtlasAppHost/Application.h"
#include "AtlasRender/Renderer.h"
#include "AtlasResource/ResourceLoader.h"
#include "bgfx/bgfx.h"
#include "Eigen/Core"
#include "AssetRegistry.h"
#include "Constants.h"
#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasRender/Types/FrameBuffer.h"

namespace
{
    struct VertexLayout
    {
        Eigen::Vector3f m_Position;
        Eigen::Vector2f m_UVs;
    };

    [[nodiscard]] bgfx::VertexBufferHandle createFullscreenQuadVertexBuffer(
        const bool originBottomLeft,
        const float width = 1.0f,
        const float height = 1.0f)
    {
        bgfx::VertexLayout vertexLayout;
        vertexLayout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();

        const float minX = -width;
        const float maxX = width;
        const float minY = -height;
        const float maxy = height;

        constexpr float minU = 0.0f;
        constexpr float maxU = 1.0f;
        float minV = 0.0f;
        float maxV = 1.0f;

        constexpr float z = 0.0f;

        if (!originBottomLeft)
        {
            std::swap(minV, maxV);
        }

        struct Vertex
        {
            Eigen::Vector3f m_Position;
            Eigen::Vector2f m_Uv;
        };

        Vertex vertices[6];
        vertices[0].m_Position = { minX, minY, z };
        vertices[0].m_Uv = { minU, minV };

        vertices[1].m_Position = { maxX, minY, z };
        vertices[1].m_Uv = { maxU, minV };

        vertices[2].m_Position = { maxX, maxy, z };
        vertices[2].m_Uv = { maxU, maxV };

        vertices[3].m_Position = { minX, minY, z };
        vertices[3].m_Uv = { minU, minV };

        vertices[4].m_Position = { maxX, maxy, z };
        vertices[4].m_Uv = { maxU, maxV };

        vertices[5].m_Position = { minX, maxy, z };
        vertices[5].m_Uv = { minU, maxV };

        static_assert(sizeof(vertices) == sizeof(Vertex) * 6);
        const bgfx::Memory* vertexMemory = bgfx::alloc(vertexLayout.getSize(6));
        std::memcpy(vertexMemory->data, vertices, vertexLayout.getSize(6));
        return bgfx::createVertexBuffer(vertexMemory, vertexLayout);
    }
}

cpp_conv::PostProcessSystem::RenderTarget::~RenderTarget()
{
    bgfx::destroy(m_Buffer);
}

void cpp_conv::PostProcessSystem::RenderTarget::Initialize(const uint32_t width, const uint32_t height,
    const bgfx::TextureFormat::Enum format, const uint64_t flags)
{
    m_Width = width;
    m_Height = height;
    m_Format = format;
    m_Flags = flags;
    m_Texture = createTexture2D(static_cast<uint16_t>(m_Width), static_cast<uint16_t>(m_Height), false, 1, m_Format, m_Flags);
    m_Buffer = createFrameBuffer(1, &m_Texture, true);

}

void cpp_conv::PostProcessSystem::Initialise(atlas::scene::EcsManager& ecsManager, const atlas::render::FrameBuffer* gbuffer)
{
    assert(gbuffer && isValid(gbuffer->GetHandle()));
    static_assert(sizeof(VertexLayout) == sizeof(float) * 3 + sizeof(float) * 2);
    m_PostProcessLayout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    m_pFrameBuffer = gbuffer;

    const bgfx::RendererType::Enum renderer = bgfx::getRendererType();
    m_TexelHalf = bgfx::RendererType::Direct3D9 == renderer ? 0.5f : 0.0f;

    m_Programs.m_Copy = atlas::resource::ResourceLoader::LoadAsset<resources::registry::CoreBundle, atlas::render::ShaderProgram>(
        resources::registry::core_bundle::shaders::postprocess::c_copy);
    m_Programs.m_Fxaa = atlas::resource::ResourceLoader::LoadAsset<resources::registry::CoreBundle, atlas::render::ShaderProgram>(
        resources::registry::core_bundle::shaders::postprocess::c_fxaa);
    m_Programs.m_Vignette = atlas::resource::ResourceLoader::LoadAsset<resources::registry::CoreBundle, atlas::render::ShaderProgram>(
        resources::registry::core_bundle::shaders::postprocess::c_vignette);

    m_Samplers.m_Color = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
    m_Uniforms.m_FrameBufferSize = bgfx::createUniform("frameBufferSize", bgfx::UniformType::Vec4);

    struct Target
    {
        std::string m_ViewName;
        bgfx::FrameBufferHandle m_FrameBuffer;
    };

    const std::array<std::string, 3> targets =
    {
        {
            "FXAA",
            "Vignette",
            "Copy",
        }
    };

    for(uint16_t i = 0; i < static_cast<uint16_t>(targets.size()); ++i)
    {
        bgfx::setViewName(constants::render_views::c_postProcess + i, targets[i].c_str());
        bgfx::setViewClear(constants::render_views::c_postProcess + i, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x322e3dFF);
        bgfx::setViewMode(constants::render_views::c_postProcess + i, bgfx::ViewMode::Sequential);
        bgfx::setViewRect(constants::render_views::c_postProcess + i, 0, 0, bgfx::BackbufferRatio::Equal);
    }

    m_FullScreenQuad = createFullscreenQuadVertexBuffer(bgfx::getCaps()->originBottomLeft);
}

void cpp_conv::PostProcessSystem::Update(atlas::scene::EcsManager& ecs)
{
    PrepareFrame();

    bgfx::ViewId view = constants::render_views::c_postProcess;
    DoFxaa(view++, Scope::InputBuffer, Scope::Interstitial);
    DoVignette(view++, Scope::Interstitial, Scope::OutputBuffer);
    //DoCopy(view++, Scope::Interstitial, Scope::OutputBuffer);
}

bgfx::TextureHandle cpp_conv::PostProcessSystem::GetInputAsTexture(const Scope target) const
{
    switch(target)
    {
    case Scope::Interstitial:
        return getTexture(m_Interstitials.m_FrameBuffer.GetHandle());
    case Scope::InputBuffer:
        return getTexture(m_pFrameBuffer->GetHandle());
    case Scope::OutputBuffer:
        break;
    }

    assert(false); // Shouldn't be here
    return BGFX_INVALID_HANDLE;
}

bgfx::FrameBufferHandle cpp_conv::PostProcessSystem::GetTargetFrameBuffer(const Scope target) const
{
    switch(target)
    {
    case Scope::Interstitial:
        return m_Interstitials.m_FrameBuffer.GetHandle();
    case Scope::InputBuffer:
        return m_pFrameBuffer->GetHandle();
    case Scope::OutputBuffer:
        return BGFX_INVALID_HANDLE;
    }

    assert(false); // Shouldn't be here
    return BGFX_INVALID_HANDLE;
}

void cpp_conv::PostProcessSystem::PrepareFrame()
{
    assert(m_Programs.m_Fxaa);
    assert(isValid(m_Programs.m_Fxaa->GetHandle()));
    assert(m_pFrameBuffer);

    auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();
    Eigen::Vector4f frameBufferSizeForUniform = { static_cast<float>(width), static_cast<float>(height), 0.0f, 0.0f };
    bgfx::setUniform(m_Uniforms.m_FrameBufferSize, frameBufferSizeForUniform.data());

    m_Interstitials.m_FrameBuffer.EnsureSize(width, height);
}

void cpp_conv::PostProcessSystem::DoFxaa(const bgfx::ViewId viewId, const Scope source, const Scope target) const
{
    bgfx::setVertexBuffer(0, m_FullScreenQuad);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_MSAA);
    bgfx::setViewFrameBuffer(viewId, GetTargetFrameBuffer(target));
    bgfx::setTexture(0, m_Samplers.m_Color, GetInputAsTexture(source));
    bgfx::submit(viewId, m_Programs.m_Fxaa->GetHandle());
}

void cpp_conv::PostProcessSystem::DoVignette(const bgfx::ViewId viewId, const Scope source, const Scope target) const
{
    bgfx::setVertexBuffer(0, m_FullScreenQuad);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_MSAA);
    bgfx::setViewFrameBuffer(viewId, GetTargetFrameBuffer(target));
    bgfx::setTexture(0, m_Samplers.m_Color, GetInputAsTexture(source));
    bgfx::submit(viewId, m_Programs.m_Vignette->GetHandle());
}

void cpp_conv::PostProcessSystem::DoCopy(const bgfx::ViewId viewId, const Scope source, const Scope target) const
{
    bgfx::setVertexBuffer(0, m_FullScreenQuad);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_MSAA);
    bgfx::setViewFrameBuffer(viewId, GetTargetFrameBuffer(target));
    bgfx::setTexture(0, m_Samplers.m_Color, GetInputAsTexture(source));
    bgfx::submit(viewId, m_Programs.m_Copy->GetHandle());
}
