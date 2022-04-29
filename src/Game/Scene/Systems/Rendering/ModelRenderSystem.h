#pragma once
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"

namespace cpp_conv
{
    class ModelRenderSystem final : public atlas::scene::SystemBase
    {
    public:
        void Initialise(atlas::scene::EcsManager&, uint8_t renderMask = 0xFF);
        void Update(atlas::scene::EcsManager& ecs) override;

    private:
        uint8_t m_RenderMask{0xFF};
    };

}
