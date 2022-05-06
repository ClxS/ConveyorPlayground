#pragma once

#include "Constants.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

class ConveyorRenderingSystem final : public atlas::scene::SystemBase
{
public:
    void Initialise(
        atlas::scene::EcsManager&,
        std::vector<uint8_t> viewIds = { cpp_conv::constants::render_views::c_geometry, cpp_conv::constants::render_views::c_shadowPass });
    void Update(atlas::scene::EcsManager& ecs) override;

    private:
        std::vector<uint8_t> m_ViewIds;
};
