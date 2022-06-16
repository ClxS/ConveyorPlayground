#pragma once

#include <vector>
#include <Eigen/Core>

#include "bgfx/bgfx.h"

#include "PolyhedronCommon.h"

namespace cpp_conv::util::geometry::polyhedron
{
    // refer to https://math.stackexchange.com/questions/2029958/numbering-a-spherical-grid-of-pentagons-and-hexagons-so-neighbours-are-easily
    class HexagonCelledPolyhedron
    {
    public:
        using Pentagon = Polygon<5>;
        using Hexagon = Polygon<6>;

    private:
        std::vector<Point> m_Points{};
        std::vector<Pentagon> m_Pentagons{};
        std::vector<Hexagon> m_Hexagons{};
    };

    HexagonCelledPolyhedron createPolyhedronHexagonal(uint32_t h, uint32_t k, float scale);
}
