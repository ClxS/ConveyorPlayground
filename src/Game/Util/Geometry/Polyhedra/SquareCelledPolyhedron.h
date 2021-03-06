#pragma once

#include <vector>
#include <Eigen/Core>

#include "bgfx/bgfx.h"

#include "PolyhedronCommon.h"

namespace cpp_conv::util::geometry::polyhedron
{
    class SquareCelledPolyhedron
    {
    public:
        using Square = Polygon<4>;
        using Triangle = Polygon<3>;

        SquareCelledPolyhedron() = default;
        SquareCelledPolyhedron(
            std::vector<Point> points,
            std::vector<Square> squares,
            std::vector<Triangle> triangles,
            const uint32_t k,
            const uint32_t h,
            const uint32_t d,
            const float radius)
                : m_Points{std::move(points)}
                , m_Squares{std::move(squares)}
                , m_Triangles{std::move(triangles)}
                , m_K{k}
                , m_H{h}
                , m_D{d}
                , m_Radius{radius}
        {
        }

        SquareCelledPolyhedron(const SquareCelledPolyhedron& other) noexcept
        {
            m_Points = other.m_Points;
            m_Squares = other.m_Squares;
            m_Triangles = other.m_Triangles;
            m_K = other.m_K;
            m_H = other.m_H;
            m_D = other.m_D;
            m_Radius = other.m_Radius;
        }

        SquareCelledPolyhedron(SquareCelledPolyhedron&& other) noexcept
        {
            m_Points = std::move(other.m_Points);
            m_Squares = std::move(other.m_Squares);
            m_Triangles = std::move(other.m_Triangles);
            m_K = other.m_K;
            m_H = other.m_H;
            m_D = other.m_D;
            m_Radius = other.m_Radius;
        }

        SquareCelledPolyhedron& operator=(const SquareCelledPolyhedron& other) noexcept
        {
            m_Points = other.m_Points;
            m_Squares = other.m_Squares;
            m_Triangles = other.m_Triangles;
            m_K = other.m_K;
            m_H = other.m_H;
            m_D = other.m_D;
            m_Radius = other.m_Radius;
            return *this;
        }

        SquareCelledPolyhedron& operator=(SquareCelledPolyhedron&& other) noexcept
        {
            m_Points = std::move(other.m_Points);
            m_Squares = std::move(other.m_Squares);
            m_Triangles = std::move(other.m_Triangles);
            m_K = other.m_K;
            m_H = other.m_H;
            m_D = other.m_D;
            m_Radius = other.m_Radius;
            return *this;
        }

        [[nodiscard]] const std::vector<Point>& GetPoints() const
        {
            return m_Points;
        }

        [[nodiscard]] const std::vector<Eigen::Vector2f>& GetSphericalCoordinates() const
        {
            return m_SphericalCoordinates;
        }

        [[nodiscard]] const std::vector<Square>& GetSquares() const
        {
            return m_Squares;
        }

        [[nodiscard]] const std::vector<Triangle>& GetTriangles() const
        {
            return m_Triangles;
        }

        [[nodiscard]] std::tuple<bgfx::VertexBufferHandle, bgfx::IndexBufferHandle> CreateBuffers() const;

        [[nodiscard]] uint32_t GetK() const { return m_K; }
        [[nodiscard]] uint32_t GetH() const { return m_H; }
        [[nodiscard]] uint32_t GetD() const { return m_D; }
        [[nodiscard]] float GetRadius() const { return m_Radius; }

    private:
        std::vector<Point> m_Points{};
        std::vector<Eigen::Vector2f> m_SphericalCoordinates{};
        std::vector<Square> m_Squares{};
        std::vector<Triangle> m_Triangles{};

        uint32_t m_K{0};
        uint32_t m_H{0};
        uint32_t m_D{0};
        float m_Radius{0.0f};
    };

    SquareCelledPolyhedron createPolyhedronSquared(uint32_t h, uint32_t k, float scale);
}
