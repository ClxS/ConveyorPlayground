#pragma once

#include <Eigen/Dense>

#include "Polyhedron.h"
#include "bgfx/bgfx.h"
#include "AtlasResource/AssetPtr.h"

namespace atlas
{
    namespace render
    {
        class ShaderProgram;
        class TextureAsset;
    }
}

namespace cpp_conv::components
{
    struct SolarBodyComponent
    {
        SolarBodyComponent() {}
        ~SolarBodyComponent()
        {
            if (isValid(m_MeshData.m_Vertices))
            {
                bgfx::destroy(m_MeshData.m_Vertices);
            }

            if (isValid(m_MeshData.m_Indices))
            {
                bgfx::destroy(m_MeshData.m_Indices);
            }
        }
        SolarBodyComponent(SolarBodyComponent&& other) noexcept
        {
            m_MeshData.m_Polyhedron = std::move(other.m_MeshData.m_Polyhedron);
            m_MeshData.m_Vertices = other.m_MeshData.m_Vertices;
            m_MeshData.m_Indices = other.m_MeshData.m_Indices;
            m_MeshData.m_Diffuse = other.m_MeshData.m_Diffuse;
            m_MeshData.m_Normal = other.m_MeshData.m_Normal;
            m_MeshData.m_Program = other.m_MeshData.m_Program;
            m_SphereData = other.m_SphereData;

            other.m_MeshData.m_Vertices = BGFX_INVALID_HANDLE;
            other.m_MeshData.m_Indices = BGFX_INVALID_HANDLE;
        }
        SolarBodyComponent& operator=(SolarBodyComponent&& other) noexcept
        {
            m_MeshData.m_Polyhedron = std::move(other.m_MeshData.m_Polyhedron);
            m_MeshData.m_Vertices = other.m_MeshData.m_Vertices;
            m_MeshData.m_Indices = other.m_MeshData.m_Indices;
            m_MeshData.m_Diffuse = other.m_MeshData.m_Diffuse;
            m_MeshData.m_Normal = other.m_MeshData.m_Normal;
            m_MeshData.m_Program = other.m_MeshData.m_Program;
            m_SphereData = other.m_SphereData;

            other.m_MeshData.m_Vertices = BGFX_INVALID_HANDLE;
            other.m_MeshData.m_Indices = BGFX_INVALID_HANDLE;
            other.m_MeshData.m_Indices = BGFX_INVALID_HANDLE;
            return *this;
        }

        struct
        {
            util::geometry::polyhedron::Polyhedron m_Polyhedron;
            bgfx::VertexBufferHandle m_Vertices{BGFX_INVALID_HANDLE};
            bgfx::IndexBufferHandle m_Indices{BGFX_INVALID_HANDLE};

            atlas::resource::AssetPtr<atlas::render::TextureAsset> m_Diffuse;
            atlas::resource::AssetPtr<atlas::render::TextureAsset> m_Normal;
            atlas::resource::AssetPtr<atlas::render::ShaderProgram> m_Program;
        } m_MeshData;

        struct
        {
            uint32_t m_K{0};
            uint32_t m_H{0};
            uint32_t m_D{0};
            float m_Radius{0.0f};
        } m_SphereData;
    };
}
