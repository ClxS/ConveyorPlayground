#include "ClippedSurfaceRenderSystem.h"

#include "AssetRegistry.h"
#include "ModelComponent.h"
#include "PositionComponent.h"
#include "AtlasRender/Renderer.h"
#include "AtlasRender/AssetTypes/ModelAsset.h"
#include "AtlasResource/ResourceLoader.h"
#include "Eigen/Core"
#include "Geometry/Polyhedra/Polyhedron.h"

namespace
{
    bgfx::VertexBufferHandle polyhedraVertices;
    bgfx::IndexBufferHandle polyhedraIndices;

    void createPolyhedra()
    {
        const auto polyhedra = cpp_conv::util::geometry::polyhedron::createPolyhedron(50, 0, 2.0f);

        bgfx::VertexLayout vertexLayout;
        vertexLayout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .end();

        const auto& points = polyhedra.GetPoints();
        const auto& triangles = polyhedra.GetTriangles();
        const auto& squares = polyhedra.GetSquares();

        const int vertexCount = static_cast<int>(points.size());
        const auto indexCount = static_cast<uint32_t>(triangles.size() * 3 + squares.size() * 6);

        const bool use16BitIndex = points.size() < 65535;
        const int indexBufferStride = use16BitIndex ? 2 : 4;

        const auto bufferSize = vertexLayout.getSize(vertexCount);
        const bgfx::Memory* vertexMemory = bgfx::alloc(bufferSize);
        const bgfx::Memory* indexMemory = bgfx::alloc(indexCount * indexBufferStride);

        std::memcpy(vertexMemory->data, points.data(), bufferSize);

        if (use16BitIndex)
        {
            int offset = 0;
            auto* indexBuffer = reinterpret_cast<uint16_t*>(indexMemory->data);
            for(const auto& square : squares)
            {
                indexBuffer[offset++] = static_cast<uint16_t>(square.m_Indices[0]);
                indexBuffer[offset++] = static_cast<uint16_t>(square.m_Indices[1]);
                indexBuffer[offset++] = static_cast<uint16_t>(square.m_Indices[2]);
                indexBuffer[offset++] = static_cast<uint16_t>(square.m_Indices[0]);
                indexBuffer[offset++] = static_cast<uint16_t>(square.m_Indices[3]);
                indexBuffer[offset++] = static_cast<uint16_t>(square.m_Indices[2]);
            }

            for(const auto& triangle : triangles)
            {
                indexBuffer[offset++] = static_cast<uint16_t>(triangle.m_Indices[0]);
                indexBuffer[offset++] = static_cast<uint16_t>(triangle.m_Indices[2]);
                indexBuffer[offset++] = static_cast<uint16_t>(triangle.m_Indices[1]);
            }
        }
        else
        {
            int offset = 0;
            auto* indexBuffer = reinterpret_cast<uint32_t*>(indexMemory->data);
            for(const auto& square : squares)
            {
                indexBuffer[offset++] = static_cast<uint32_t>(square.m_Indices[0]);
                indexBuffer[offset++] = static_cast<uint32_t>(square.m_Indices[1]);
                indexBuffer[offset++] = static_cast<uint32_t>(square.m_Indices[2]);
                indexBuffer[offset++] = static_cast<uint32_t>(square.m_Indices[0]);
                indexBuffer[offset++] = static_cast<uint32_t>(square.m_Indices[3]);
                indexBuffer[offset++] = static_cast<uint32_t>(square.m_Indices[2]);
            }

            for(const auto& triangle : triangles)
            {
                indexBuffer[offset++] = static_cast<uint32_t>(triangle.m_Indices[0]);
                indexBuffer[offset++] = static_cast<uint32_t>(triangle.m_Indices[2]);
                indexBuffer[offset++] = static_cast<uint32_t>(triangle.m_Indices[1]);
            }
        }

        polyhedraVertices = bgfx::createVertexBuffer(vertexMemory, vertexLayout);
        polyhedraIndices = bgfx::createIndexBuffer(indexMemory, use16BitIndex ? BGFX_BUFFER_NONE : BGFX_BUFFER_INDEX32);
    }
}

void cpp_conv::ClippedSurfaceRenderSystem::Initialise(atlas::scene::EcsManager&,
                                                      const uint8_t surfaceRenderMask,
                                                      const uint8_t clipCasterRenderMask)
{
    createPolyhedra();


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

    bgfx::setState(0
                    | BGFX_STATE_WRITE_RGB
                    | BGFX_STATE_WRITE_A
                    | BGFX_STATE_WRITE_Z
                    | BGFX_STATE_DEPTH_TEST_LESS
                    | BGFX_STATE_FRONT_CCW
                    | BGFX_STATE_MSAA);
    draw(
        constants::render_views::c_geometry,
        polyhedraVertices,
        polyhedraIndices,
        atlas::resource::ResourceLoader::LoadAsset<resources::registry::CoreBundle, atlas::render::ShaderProgram>(resources::registry::core_bundle::shaders::c_basicUntexturedUninstanced),
        Eigen::Matrix4f::Identity(),
        ~BGFX_DISCARD_STATE);
}
