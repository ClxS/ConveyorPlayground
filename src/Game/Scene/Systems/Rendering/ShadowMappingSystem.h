#pragma once

#include <array>

#include "AtlasResource/AssetPtr.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"
#include "bgfx/bgfx.h"

namespace atlas
{
    namespace render
    {
        class ShaderProgram;
    }
}

namespace cpp_conv
{
    class ShadowMappingSystem final : public atlas::scene::SystemBase
    {
    public:
        void Initialise(atlas::scene::EcsManager&, uint16_t shadowMapWidth = 2048, uint16_t shadowMapHeight = 2048);

        void Update(atlas::scene::EcsManager& ecs) override;

    private:
        uint16_t m_ShadowMapWidth{512};
        uint16_t m_ShadowMapHeight{512};

        struct
        {
            atlas::resource::AssetPtr<atlas::render::ShaderProgram> m_ShadowMap;
        } m_Programs;

        bgfx::FrameBufferHandle m_ShadowMapFrameBuffer{BGFX_INVALID_HANDLE};
    };
}
