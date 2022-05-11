#pragma once

#include "AssetRegistry.h"
#include "Constants.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/ResourceLoader.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace atlas
{
    namespace render
    {
        class ShaderProgram;
    }
}

class ConveyorRenderingSystem final : public atlas::scene::SystemBase
{
public:
    struct Pass
    {
        uint8_t m_ViewId;
        atlas::resource::AssetPtr<atlas::render::ShaderProgram> m_bOverrideProgram;
    };
    void Initialise(
        atlas::scene::EcsManager&,
        std::vector<Pass> viewIds =
        {
            {
                cpp_conv::constants::render_views::c_geometry,
                nullptr
            },
            {
                cpp_conv::constants::render_views::c_shadowPass,
                atlas::resource::ResourceLoader::LoadAsset<
                    cpp_conv::resources::registry::CoreBundle,
                    atlas::render::ShaderProgram>(cpp_conv::resources::registry::core_bundle::shaders::c_shadowMapBasic)
            }
        });
    void Update(atlas::scene::EcsManager& ecs) override;

    private:
        std::vector<Pass> m_Passes;
};
