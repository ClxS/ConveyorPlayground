#pragma once
#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"
#include "bgfx/bgfx.h"

namespace atlas
{
    namespace render
    {
        class FrameBuffer;
    }
}

namespace cpp_conv
{
    class PostProcessSystem final : public atlas::scene::SystemBase
    {
        struct RenderTarget
        {
            ~RenderTarget();

            void Initialize(const uint32_t width, const uint32_t height, const bgfx::TextureFormat::Enum format, const uint64_t flags);

            uint32_t m_Width{};
            uint32_t m_Height{};
            bgfx::TextureFormat::Enum m_Format{};
            uint64_t m_Flags{};
            bgfx::TextureHandle m_Texture{};
            bgfx::FrameBufferHandle m_Buffer{};
        };
    public:
        void Initialise(atlas::scene::EcsManager&, const atlas::render::FrameBuffer* gbuffer);

        void Update(atlas::scene::EcsManager& ecs) override;

    private:
        bgfx::VertexLayout m_PostProcessLayout{};

        const atlas::render::FrameBuffer* m_pFrameBuffer;

        struct
        {
            atlas::resource::AssetPtr<atlas::render::ShaderProgram> m_Fxaa{};
            atlas::resource::AssetPtr<atlas::render::ShaderProgram> m_Copy{};
        } m_Programs;

        struct
        {
            bgfx::UniformHandle m_Color{BGFX_INVALID_HANDLE};
        } m_Samplers;

        struct
        {
            bgfx::UniformHandle m_FrameBufferSize{BGFX_INVALID_HANDLE};
        } m_Uniforms;

        float m_TexelHalf{};
    };
}
