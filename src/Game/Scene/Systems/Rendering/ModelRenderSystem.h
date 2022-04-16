#pragma once
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class ModelRenderSystem final : public atlas::scene::SystemBase
    {
    public:
        void Update(atlas::scene::EcsManager& ecs) override;
    };

}
