#pragma once

#include "Constants.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

class ConveyorRenderingSystem final : public atlas::scene::SystemBase
{
public:
    void Initialise(
        atlas::scene::EcsManager&,
        uint8_t viewId = cpp_conv::constants::render_views::c_geometry);
    void Update(atlas::scene::EcsManager& ecs) override;

    private:
        uint8_t m_ViewId{cpp_conv::constants::render_views::c_geometry};
};
