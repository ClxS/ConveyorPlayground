#pragma once

#include <Eigen/Dense>

#include "HexagonCelledPolyhedron.h"
#include "SquareCelledPolyhedron.h"
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
    struct SolarBodyMeshData
    {
        bgfx::VertexBufferHandle m_Vertices{BGFX_INVALID_HANDLE};
        bgfx::IndexBufferHandle m_Indices{BGFX_INVALID_HANDLE};

        atlas::resource::AssetPtr<atlas::render::TextureAsset> m_Diffuse;
        atlas::resource::AssetPtr<atlas::render::TextureAsset> m_Normal;
        atlas::resource::AssetPtr<atlas::render::ShaderProgram> m_Program;
    };

    template<typename TPolyhedronType>
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
        SolarBodyComponent(const SolarBodyComponent& other) = delete;
        SolarBodyComponent(SolarBodyComponent&& other) noexcept
        {
            m_MeshData.m_Vertices = other.m_MeshData.m_Vertices;
            m_MeshData.m_Indices = other.m_MeshData.m_Indices;
            m_MeshData.m_Diffuse = other.m_MeshData.m_Diffuse;
            m_MeshData.m_Normal = other.m_MeshData.m_Normal;
            m_MeshData.m_Program = other.m_MeshData.m_Program;
            m_SphereData.m_Polyhedron = std::move(other.m_SphereData.m_Polyhedron);
            m_SphereData.m_K = other.m_SphereData.m_K;
            m_SphereData.m_H = other.m_SphereData.m_H;
            m_SphereData.m_D = other.m_SphereData.m_D;
            m_SphereData.m_Radius = other.m_SphereData.m_Radius;

            other.m_MeshData.m_Vertices = BGFX_INVALID_HANDLE;
            other.m_MeshData.m_Indices = BGFX_INVALID_HANDLE;
        }
        SolarBodyComponent& operator=(const SolarBodyComponent& other) = delete;
        SolarBodyComponent& operator=(SolarBodyComponent&& other) noexcept
        {
            m_MeshData.m_Vertices = other.m_MeshData.m_Vertices;
            m_MeshData.m_Indices = other.m_MeshData.m_Indices;
            m_MeshData.m_Diffuse = other.m_MeshData.m_Diffuse;
            m_MeshData.m_Normal = other.m_MeshData.m_Normal;
            m_MeshData.m_Program = other.m_MeshData.m_Program;

            m_SphereData.m_Polyhedron = std::move(other.m_SphereData.m_Polyhedron);
            m_SphereData.m_K = other.m_SphereData.m_K;
            m_SphereData.m_H = other.m_SphereData.m_H;
            m_SphereData.m_D = other.m_SphereData.m_D;
            m_SphereData.m_Radius = other.m_SphereData.m_Radius;

            other.m_MeshData.m_Vertices = BGFX_INVALID_HANDLE;
            other.m_MeshData.m_Indices = BGFX_INVALID_HANDLE;
            other.m_MeshData.m_Indices = BGFX_INVALID_HANDLE;
            return *this;
        }

        SolarBodyMeshData m_MeshData;

        struct
        {
            TPolyhedronType m_Polyhedron;
            uint32_t m_K{0};
            uint32_t m_H{0};
            uint32_t m_D{0};
            float m_Radius{0.0f};
        } m_SphereData;
    };

    using SquareSolarBodyComponent = SolarBodyComponent<util::geometry::polyhedron::SquareCelledPolyhedron>;
    using HexagonSolarBodyComponent = SolarBodyComponent<util::geometry::polyhedron::HexagonCelledPolyhedron>;
}
