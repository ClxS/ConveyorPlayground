#include "ClippedSurfaceRenderSystem.h"

#include "ModelComponent.h"
#include "PositionComponent.h"
#include "AtlasRender/Renderer.h"
#include "AtlasRender/AssetTypes/ModelAsset.h"
#include "Eigen/Core"

void cpp_conv::ClippedSurfaceRenderSystem::Initialise(atlas::scene::EcsManager&,
                                                      const uint8_t surfaceRenderMask,
                                                      const uint8_t clipCasterRenderMask)
{
    m_Masks.m_SurfaceRenderMask = surfaceRenderMask;
    m_Masks.m_ClipCasterRenderMask = clipCasterRenderMask;
    m_Uniforms.m_SurfaceColour = createUniform("u_surfaceColour", bgfx::UniformType::Vec4);
    m_Uniforms.m_ClipCount = createUniform("u_clipCount", bgfx::UniformType::Vec4);
    m_Uniforms.m_WorldClipPositions = createUniform("u_worldClipPositions", bgfx::UniformType::Vec4);
    m_Uniforms.m_WorldClipRadii = createUniform("u_worldClipRadii", bgfx::UniformType::Vec4);
}

void cpp_conv::ClippedSurfaceRenderSystem::Update(atlas::scene::EcsManager& ecs)
{
    // TODO Replace this with a clip mask system, currently this only allows clipping circular clippings
    std::array<Eigen::Vector4f, c_maxClipPositions> clipPositions;
    std::array<Eigen::Vector4f, c_maxClipPositions> clipRanges;

    uint16_t clipCount = 0;
    for(auto [entity, model, position] : ecs.IterateEntityComponents<components::ModelComponent, components::PositionComponent>())
    {
        if ((model.m_RenderMask & m_Masks.m_ClipCasterRenderMask) != m_Masks.m_ClipCasterRenderMask || !model.m_Model || !model.m_Model->GetMesh() || !model.m_Model->GetProgram())
        {
            continue;
        }

        auto floatPosition = position.m_Position.cast<float>();
        clipPositions[clipCount] = { floatPosition.x(), floatPosition.y(), floatPosition.z(), 0.0f };

        float sphereRadius = 0.0f;
        for(auto& segment : model.m_Model->GetMesh()->GetSegments())
        {
            sphereRadius = std::max(sphereRadius, segment.m_Bounds.m_Sphere.radius);
        }

        clipRanges[clipCount] = { sphereRadius, 0.0f, 0.0f, 0.0f };
        clipCount++;
    }

    // Annoying bgfx only really supports vec4s... We have to pretend to be one.
    Eigen::Vector4f vecColour = { 150.0f / 255.0f, 117.0f / 255.0f, 66.0f / 255.0f, 1.0f };
    Eigen::Vector4f vecClipCount = { static_cast<float>(clipCount), 0.0f, 0.0f, 0.0f };

    bgfx::setUniform(m_Uniforms.m_SurfaceColour, vecColour.data(), 1);
    bgfx::setUniform(m_Uniforms.m_ClipCount, vecClipCount.data(), 1);
    bgfx::setUniform(m_Uniforms.m_WorldClipPositions, clipPositions.data(), clipCount);
    bgfx::setUniform(m_Uniforms.m_WorldClipRadii, clipRanges.data(), clipCount);

    for(auto [entity, model, position] : ecs.IterateEntityComponents<components::ModelComponent, components::PositionComponent>())
    {
        if ((model.m_RenderMask & m_Masks.m_SurfaceRenderMask) != m_Masks.m_SurfaceRenderMask || !model.m_Model || !model.m_Model->GetMesh() || !model.m_Model->GetProgram())
        {
            continue;
        }

        draw(
            constants::render_views::c_geometry,
            model.m_Model,
            model.m_Model->GetProgram(),
            Eigen::Matrix4f::Identity(),
            ~BGFX_DISCARD_STATE);
    }
}
