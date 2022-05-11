#pragma once

#include <array>

#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"
#include "bgfx/bgfx.h"

namespace cpp_conv
{
    constexpr uint8_t c_MaxDirectionalLights = 1;

    class LightingRenderSystem final : public atlas::scene::SystemBase
    {
    public:
        void Initialise(atlas::scene::EcsManager&) override;
        void Update(atlas::scene::EcsManager& ecs) override;

    private:
        struct
        {
            std::array<bgfx::UniformHandle, c_MaxDirectionalLights> m_LightDirections;
            std::array<bgfx::UniformHandle, c_MaxDirectionalLights> m_LightColours;
            bgfx::UniformHandle m_AmbientColour{BGFX_INVALID_HANDLE};
        } m_Uniforms{};
    };
}
