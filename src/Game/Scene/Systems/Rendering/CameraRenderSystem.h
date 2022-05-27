#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class CameraRenderSystem final : public atlas::scene::SystemBase
    {
    public:
        void Initialise(atlas::scene::EcsManager&) override;
        void Update(atlas::scene::EcsManager& ecs) override;

        void SetDebugRenderingEnabled(bool bEnabled);
    private:
        bool m_bDebugRenderingEnabled{true};
    };

}
