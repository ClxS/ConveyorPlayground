#pragma once

#include <vector>
#include <Eigen/Core>

#include "bgfx/bgfx.h"

namespace cpp_conv::util::geometry::polyhedron
{
    class Polyhedron
    {
    public:
        struct Point
        {
            float m_X;
            float m_Y;
            float m_Z;

            Point operator+(const Point& other) const
            {
                return { m_X + other.m_X, m_Y + other.m_Y, m_Z + other.m_Z };
            }

            Point operator-(const Point& other) const
            {
                return { m_X - other.m_X, m_Y - other.m_Y, m_Z - other.m_Z };
            }

            Point& operator+=(const Point& other)
            {
                *this = *this + other;
                return *this;
            }

            Point& operator-=(const Point& other)
            {
                *this = *this - other;
                return *this;
            }
        };

        template<uint32_t TIndexCount>
        struct Polygon
        {
            Eigen::Matrix<int, TIndexCount, 1, 0> m_Indices{};
        };

        using Square = Polygon<4>;
        using Triangle = Polygon<3>;

        Polyhedron()
        {
        }

        Polyhedron(
            std::vector<Point> points,
            std::vector<Square> squares,
            std::vector<Triangle> triangles)
                : m_Points{std::move(points)}
                , m_Squares{std::move(squares)}
                , m_Triangles{std::move(triangles)}
        {
        }

        Polyhedron(const Polyhedron& other) noexcept
        {
            m_Points = other.m_Points;
            m_Squares = other.m_Squares;
            m_Triangles = other.m_Triangles;
        }

        Polyhedron(Polyhedron&& other) noexcept
        {
            m_Points = std::move(other.m_Points);
            m_Squares = std::move(other.m_Squares);
            m_Triangles = std::move(other.m_Triangles);
        }

        Polyhedron& operator=(const Polyhedron& other) noexcept
        {
            m_Points = other.m_Points;
            m_Squares = other.m_Squares;
            m_Triangles = other.m_Triangles;
            return *this;
        }

        Polyhedron& operator=(Polyhedron&& other) noexcept
        {
            m_Points = std::move(other.m_Points);
            m_Squares = std::move(other.m_Squares);
            m_Triangles = std::move(other.m_Triangles);
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

    private:
        std::vector<Point> m_Points;
        std::vector<Eigen::Vector2f> m_SphericalCoordinates;
        std::vector<Square> m_Squares;
        std::vector<Triangle> m_Triangles;
    };

    Polyhedron createPolyhedron(uint32_t h, uint32_t k, float scale);
}
