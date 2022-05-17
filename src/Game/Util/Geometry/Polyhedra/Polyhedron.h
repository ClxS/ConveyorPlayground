#pragma once

#include <vector>
#include <Eigen/Core>

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
        };

        template<uint32_t TIndexCount>
        struct Polygon
        {
            Eigen::Matrix<int, TIndexCount, 1, 0> m_Indices{};
        };

        using Square = Polygon<4>;
        using Triangle = Polygon<3>;

        Polyhedron(
            std::vector<Point> points,
            std::vector<Square> squares,
            std::vector<Triangle> triangles)
                : m_Points{std::move(points)}
                , m_Squares{std::move(squares)}
                , m_Triangles{std::move(triangles)}
        {
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

    private:
        std::vector<Point> m_Points;
        std::vector<Eigen::Vector2f> m_SphericalCoordinates;
        std::vector<Square> m_Squares;
        std::vector<Triangle> m_Triangles;
    };

    Polyhedron createPolyhedron(uint32_t h, uint32_t k, float scale);
}
