#pragma once
#include "AtlasScene/ECS/Systems/SystemBase.h"
#include "bgfx/bgfx.h"

namespace atlas::game::scene::systems::cameras
{
    class CameraViewProjectionUpdateSystem final : public atlas::scene::SystemBase
    {
    public:
        explicit CameraViewProjectionUpdateSystem(const bgfx::ViewId viewId) : m_ViewId{viewId} {}
        void Initialise(atlas::scene::EcsManager&) override;
        void Update(atlas::scene::EcsManager& ecs) override;

        void SetDebugRenderingEnabled(bool bEnabled);
    private:
        bgfx::ViewId m_ViewId;
        bool m_bDebugRenderingEnabled{true};
    };
}
