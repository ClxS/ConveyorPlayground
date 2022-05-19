#pragma once
#include "Constants.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasScene/ECS/Systems/SystemBase.h"
#include "bgfx/bgfx.h"

namespace cpp_conv
{
    constexpr int c_maxClipPositions = 32;

    class SolarBodyRenderSystem final : public atlas::scene::SystemBase
    {
    public:
        void Initialise(
            atlas::scene::EcsManager&,
            uint8_t surfaceRenderMask = constants::render_masks::c_surfaceClippedGeometry,
            uint8_t clipCasterRenderMask = constants::render_masks::c_clipCasterGeometry);

        void Update(atlas::scene::EcsManager& ecs) override;

    private:
        struct
        {
            uint8_t m_SurfaceRenderMask{constants::render_masks::c_surfaceClippedGeometry};
            uint8_t m_ClipCasterRenderMask{constants::render_masks::c_clipCasterGeometry};
        } m_Masks;

        struct
        {
            bgfx::UniformHandle m_SurfaceColour{BGFX_INVALID_HANDLE};
            bgfx::UniformHandle m_ClipCount{BGFX_INVALID_HANDLE};
            bgfx::UniformHandle m_WorldClipPositions{BGFX_INVALID_HANDLE};
            bgfx::UniformHandle m_WorldClipRadii{BGFX_INVALID_HANDLE};
        } m_Uniforms;

        struct
        {
            bgfx::UniformHandle m_Color{BGFX_INVALID_HANDLE};
            bgfx::UniformHandle m_Normal{BGFX_INVALID_HANDLE};
        } m_Samplers;
    };

}
