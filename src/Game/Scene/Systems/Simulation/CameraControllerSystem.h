#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class EntityLookupGrid;

    class CameraControllerSystem final : public atlas::scene::SystemBase
    {
    public:
        void Update(atlas::scene::EcsManager&) override;
    };
}
