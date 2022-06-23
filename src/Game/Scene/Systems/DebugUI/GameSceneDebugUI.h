#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace atlas::game::scene::systems::cameras
{
    class CameraViewProjectionUpdateSystem;
}

namespace cpp_conv
{
    class GameSceneDebugUI final : public atlas::scene::SystemBase
    {
    public:
        void Initialise(atlas::scene::EcsManager&, atlas::game::scene::systems::cameras::CameraViewProjectionUpdateSystem*);
        void Update(atlas::scene::EcsManager& ecs) override;

    private:
        atlas::game::scene::systems::cameras::CameraViewProjectionUpdateSystem* m_pCameraRenderer{nullptr};
    };

}
