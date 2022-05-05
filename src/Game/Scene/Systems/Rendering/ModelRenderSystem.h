#pragma once
#include "Constants.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class ModelRenderSystem final : public atlas::scene::SystemBase
    {
    public:
        void Initialise(
            atlas::scene::EcsManager&,
            uint8_t viewId = constants::render_views::c_geometry,
            uint8_t renderMask = constants::render_masks::c_generalGeometry);
        void Update(atlas::scene::EcsManager& ecs) override;

    private:
        uint8_t m_ViewId{constants::render_views::c_geometry};
        uint8_t m_RenderMask{constants::render_masks::c_generalGeometry};
    };

}
