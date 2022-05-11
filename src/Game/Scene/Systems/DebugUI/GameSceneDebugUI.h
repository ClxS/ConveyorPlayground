#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class GameSceneDebugUI final : public atlas::scene::SystemBase
    {
    public:
        void Initialise(atlas::scene::EcsManager&) override;
        void Update(atlas::scene::EcsManager& ecs) override;
    };

}
