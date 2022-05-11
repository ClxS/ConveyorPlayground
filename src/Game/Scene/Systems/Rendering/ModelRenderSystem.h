#pragma once
#include "AssetRegistry.h"
#include "Constants.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/ResourceLoader.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"
#include "bgfx/defines.h"

namespace atlas
{
    namespace render
    {
        class ShaderProgram;
    }
}

namespace cpp_conv
{
    class ModelRenderSystem final : public atlas::scene::SystemBase
    {
    public:
        struct Pass
        {
            uint8_t m_ViewId;
            uint8_t m_RenderMask;
            uint64_t m_State;
            atlas::resource::AssetPtr<atlas::render::ShaderProgram> m_bOverrideProgram;
        };

        void Initialise(
            atlas::scene::EcsManager&,
            std::vector<Pass> passes = {
                {
                    constants::render_views::c_geometry,
                    constants::render_masks::c_generalGeometry,
                    BGFX_STATE_DEFAULT,
                    nullptr
                },
                {
                    constants::render_views::c_shadowPass,
                    constants::render_masks::c_shadowCaster,
                    0
                    | BGFX_STATE_WRITE_RGB
                    | BGFX_STATE_WRITE_A
                    | BGFX_STATE_WRITE_Z
                    | BGFX_STATE_DEPTH_TEST_LESS
                    | BGFX_STATE_CULL_CCW
                    | BGFX_STATE_MSAA,
                atlas::resource::ResourceLoader::LoadAsset<
                    cpp_conv::resources::registry::CoreBundle,
                    atlas::render::ShaderProgram>(cpp_conv::resources::registry::core_bundle::shaders::c_shadowMapBasic)
                },
            });

        void Update(atlas::scene::EcsManager& ecs) override;

    private:
        std::vector<Pass> m_Passes;
    };

}
