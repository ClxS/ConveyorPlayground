#pragma once
#include "Constants.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class ModelRenderSystem final : public atlas::scene::SystemBase
    {
    public:
        struct Pass
        {
            uint8_t m_ViewId;
            uint8_t m_RenderMask;
        };

        void Initialise(
            atlas::scene::EcsManager&,
            std::vector<Pass> passes = {
                { constants::render_views::c_geometry, constants::render_masks::c_generalGeometry },
                { constants::render_views::c_shadowPass, constants::render_masks::c_shadowCaster },
            });

        void Update(atlas::scene::EcsManager& ecs) override;

    private:
        std::vector<Pass> m_Passes;
    };

}
