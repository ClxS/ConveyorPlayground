#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class CameraRenderSystem;

    class GameSceneDebugUI final : public atlas::scene::SystemBase
    {
    public:
        void Initialise(atlas::scene::EcsManager&, CameraRenderSystem*);
        void Update(atlas::scene::EcsManager& ecs) override;

    private:
        CameraRenderSystem* m_pCameraRenderer{nullptr};
    };

}
