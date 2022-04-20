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

    bool setVertexBufferToFullscreenQuad(
        const bgfx::VertexLayout& layout,
        const float textureWidth,
        const float textureHeight,
        const float texelHalf,
        const bool originBottomLeft,
        const float width = 1.0f,
        const float height = 1.0f)
    {
        if (getAvailTransientVertexBuffer(6, layout) != 6)
        {
            return false;
        }

        bgfx::TransientVertexBuffer vb{};
        bgfx::allocTransientVertexBuffer(&vb, 6, layout);
        const auto vertex = reinterpret_cast<VertexLayout*>(vb.data);

        const float minX = -width;
        const float maxX = width;
        const float minY = -height;
        const float maxy = height;

        const float texelHalfW = texelHalf / textureWidth;
        const float texelHalfH = texelHalf / textureHeight;

        const float minU = 0.0f;
        const float maxU = 1.0f;
        float minV = 0.0f;
        float maxV = 1.0f;

        constexpr float z = 0.0f;

        if (!originBottomLeft)
        {
            std::swap(minV, maxV);
        }

        vertex[0].m_Position = { minX, minY, z };
        vertex[0].m_UVs = { minU, minV };

        vertex[1].m_Position = { maxX, minY, z };
        vertex[1].m_UVs = { maxU, minV };

        vertex[2].m_Position = { maxX, maxy, z };
        vertex[2].m_UVs = { maxU, maxV };

        vertex[3].m_Position = { minX, minY, z };
        vertex[3].m_UVs = { minU, minV };

        vertex[4].m_Position = { maxX, maxy, z };
        vertex[4].m_UVs = { maxU, maxV };

        vertex[5].m_Position = { minX, maxy, z };
        vertex[5].m_UVs = { minU, maxV };

        setVertexBuffer(0, &vb);
        return true;
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
    static_assert(sizeof(VertexLayout) == sizeof(float) * 3 + sizeof(float) * 2);
    m_PostProcessLayout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    m_pFrameBuffer = gbuffer;

    const bgfx::RendererType::Enum renderer = bgfx::getRendererType();
    m_TexelHalf = bgfx::RendererType::Direct3D9 == renderer ? 0.5f : 0.0f;

    auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();
    m_Programs.m_Copy = atlas::resource::ResourceLoader::LoadAsset<resources::registry::CoreBundle, atlas::render::ShaderProgram>(resources::registry::core_bundle::shaders::postprocess::c_copy);
    m_Programs.m_Fxaa = atlas::resource::ResourceLoader::LoadAsset<resources::registry::CoreBundle, atlas::render::ShaderProgram>(resources::registry::core_bundle::shaders::postprocess::c_fxaa);

    m_Samplers.m_Color = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler); // Color (albedo) gbuffer, default color input
    m_Uniforms.m_FrameBufferSize = bgfx::createUniform("frameBufferSize", bgfx::UniformType::Vec4);
}

void cpp_conv::PostProcessSystem::Update(atlas::scene::EcsManager& ecs)
{
    if (!m_Programs.m_Fxaa || !isValid(m_Programs.m_Fxaa->GetHandle()) || !m_pFrameBuffer)
    {
        return;
    }

    auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();

    Eigen::Vector4f frameBufferSize = { static_cast<float>(width), static_cast<float>(height), 0.0f, 0.0f };

    bgfx::setUniform(m_Uniforms.m_FrameBufferSize, frameBufferSize.data());
    setVertexBufferToFullscreenQuad(m_PostProcessLayout, static_cast<float>(width), static_cast<float>(height), m_TexelHalf, bgfx::getCaps()->originBottomLeft);

    bgfx::setTexture(0, m_Samplers.m_Color, bgfx::getTexture(m_pFrameBuffer->GetHandle()));

    bgfx::submit(constants::render_views::c_postProcess, m_Programs.m_Fxaa->GetHandle());
}
