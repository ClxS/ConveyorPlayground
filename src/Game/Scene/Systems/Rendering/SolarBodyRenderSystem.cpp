#include "SolarBodyRenderSystem.h"

#include "AssetRegistry.h"
#include "ModelComponent.h"
#include "PositionComponent.h"
#include "SolarBodyComponent.h"
#include "AtlasRender/Renderer.h"
#include "AtlasRender/AssetTypes/ModelAsset.h"
#include "AtlasRender/Debug/debugdraw.h"
#include "AtlasResource/ResourceLoader.h"
#include "Eigen/Core"
#include "Geometry/Polyhedra/Polyhedron.h"

void cpp_conv::SolarBodyRenderSystem::Initialise(atlas::scene::EcsManager&,
                                                      const uint8_t surfaceRenderMask,
                                                      const uint8_t clipCasterRenderMask)
{
    m_Masks.m_SurfaceRenderMask = surfaceRenderMask;
    m_Masks.m_ClipCasterRenderMask = clipCasterRenderMask;
    m_Uniforms.m_SurfaceColour = createUniform("u_surfaceColour", bgfx::UniformType::Vec4);
    m_Uniforms.m_ClipCount = createUniform("u_clipCount", bgfx::UniformType::Vec4);
    m_Uniforms.m_WorldClipPositions = createUniform("u_worldClipPositions", bgfx::UniformType::Vec4);
    m_Uniforms.m_WorldClipRadii = createUniform("u_worldClipRadii", bgfx::UniformType::Vec4);

    m_Samplers.m_Color = bgfx::createUniform("s_diffuse", bgfx::UniformType::Sampler);
    m_Samplers.m_Normal = bgfx::createUniform("s_normal", bgfx::UniformType::Sampler);
}

void cpp_conv::SolarBodyRenderSystem::Update(atlas::scene::EcsManager& ecs)
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

    for(auto [entity, solarBody] : ecs.IterateEntityComponents<components::SolarBodyComponent>())
    {
        bgfx::setState(0
                        | BGFX_STATE_WRITE_RGB
                        | BGFX_STATE_WRITE_A
                        | BGFX_STATE_WRITE_Z
                        | BGFX_STATE_DEPTH_TEST_LESS
                        | BGFX_STATE_FRONT_CCW
                        | BGFX_STATE_MSAA);

        bgfx::setTexture(0, m_Samplers.m_Color, solarBody.m_MeshData.m_Diffuse->GetHandle());
        bgfx::setTexture(1, m_Samplers.m_Normal, solarBody.m_MeshData.m_Normal->GetHandle());
        draw(
            constants::render_views::c_geometry,
            solarBody.m_MeshData.m_Vertices,
            solarBody.m_MeshData.m_Indices,
            solarBody.m_MeshData.m_Program,
            Eigen::Matrix4f::Identity(),
            ~BGFX_DISCARD_STATE);

        {
            using namespace atlas::render::debug;
            debug_draw::createScope();
            debug_draw::setWireframe(true);

            const auto& points = solarBody.m_MeshData.m_Polyhedron.GetPoints();

            debug_draw::setColor(0xff000000);
            for(const auto& square : solarBody.m_MeshData.m_Polyhedron.GetSquares())
            {
                const auto& v0 = points[square.m_Indices[0]];
                const auto& v1 = points[square.m_Indices[1]];
                const auto& v2 = points[square.m_Indices[2]];
                const auto& v3 = points[square.m_Indices[3]];

                debug_draw::moveTo({ v0.m_X, v0.m_Y, v0.m_Z });
                debug_draw::lineTo({ v1.m_X, v1.m_Y, v1.m_Z });

                debug_draw::moveTo({ v1.m_X, v1.m_Y, v1.m_Z });
                debug_draw::lineTo({ v2.m_X, v2.m_Y, v2.m_Z });

                debug_draw::moveTo({ v2.m_X, v2.m_Y, v2.m_Z });
                debug_draw::lineTo({ v3.m_X, v3.m_Y, v3.m_Z });

                debug_draw::moveTo({ v3.m_X, v3.m_Y, v3.m_Z });
                debug_draw::lineTo({ v0.m_X, v0.m_Y, v0.m_Z });
            }

            for(const auto& triangle : solarBody.m_MeshData.m_Polyhedron.GetTriangles())
            {
                const auto& v0 = points[triangle.m_Indices[0]];
                const auto& v1 = points[triangle.m_Indices[1]];
                const auto& v2 = points[triangle.m_Indices[2]];

                debug_draw::moveTo({ v0.m_X, v0.m_Y, v0.m_Z });
                debug_draw::lineTo({ v1.m_X, v1.m_Y, v1.m_Z });

                debug_draw::moveTo({ v0.m_X, v0.m_Y, v0.m_Z });
                debug_draw::lineTo({ v1.m_X, v1.m_Y, v1.m_Z });

                debug_draw::moveTo({ v0.m_X, v0.m_Y, v0.m_Z });
                debug_draw::lineTo({ v1.m_X, v1.m_Y, v1.m_Z });
            }
        }
    }
}
