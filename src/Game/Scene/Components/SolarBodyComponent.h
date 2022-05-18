#pragma once

#include <Eigen/Dense>

#include "Polyhedron.h"
#include "bgfx/bgfx.h"

namespace cpp_conv::components
{
    struct SolarBodyComponent
    {
        SolarBodyComponent() {}
        ~SolarBodyComponent()
        {
            bgfx::destroy(m_MeshData.m_Vertices);
            bgfx::destroy(m_MeshData.m_Indices);
        }
        SolarBodyComponent(const SolarBodyComponent& other) noexcept
        {
            m_MeshData.m_Polyhedron = other.m_MeshData.m_Polyhedron;
            m_MeshData.m_Vertices = other.m_MeshData.m_Vertices;
            m_MeshData.m_Indices = other.m_MeshData.m_Indices;
            m_SphereData = other.m_SphereData;
        }
        SolarBodyComponent(SolarBodyComponent&& other) noexcept
        {
            m_MeshData.m_Polyhedron = std::move(other.m_MeshData.m_Polyhedron);
            m_MeshData.m_Vertices = other.m_MeshData.m_Vertices;
            m_MeshData.m_Indices = other.m_MeshData.m_Indices;
            m_SphereData = other.m_SphereData;
        }

        SolarBodyComponent& operator=(const SolarBodyComponent& other) noexcept
        {
            m_MeshData.m_Polyhedron = other.m_MeshData.m_Polyhedron;
            m_MeshData.m_Vertices = other.m_MeshData.m_Vertices;
            m_MeshData.m_Indices = other.m_MeshData.m_Indices;
            m_SphereData = other.m_SphereData;
            return *this;
        }
        SolarBodyComponent& operator=(SolarBodyComponent&& other) noexcept
        {
            m_MeshData.m_Polyhedron = std::move(other.m_MeshData.m_Polyhedron);
            m_MeshData.m_Vertices = other.m_MeshData.m_Vertices;
            m_MeshData.m_Indices = other.m_MeshData.m_Indices;
            m_SphereData = other.m_SphereData;
            return *this;
        }

        struct
        {
            util::geometry::polyhedron::Polyhedron m_Polyhedron;
            bgfx::VertexBufferHandle m_Vertices{BGFX_INVALID_HANDLE};
            bgfx::IndexBufferHandle m_Indices{BGFX_INVALID_HANDLE};
        } m_MeshData;

        struct
        {
            uint32_t m_K{0};
            uint32_t m_H{0};
            uint32_t m_D{0};
        } m_SphereData;
    };
}
