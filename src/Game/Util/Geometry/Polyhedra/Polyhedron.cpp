// This is based on https://github.com/eduidl/tgp-ceres (Apache 2.0).
// It has been very heavily modified

#include "Polyhedron.h"

#include <cassert>
#include <cmath>
#include <optional>
#include <tuple>
#include <vector>

#include "Eigen/Geometry"

namespace polyhedron
{
    class TempSphericalCoordinate final
    {
    public:
        TempSphericalCoordinate() = default;
        TempSphericalCoordinate(size_t id, uint32_t x, uint32_t y, uint32_t segmentIndex, uint32_t d);

        [[nodiscard]] std::tuple<float, float> GetParams() const { return std::make_tuple(m_Theta, m_Phi); }

        void SetParams(const float theta, const float phi)
        {
            m_Theta = theta;
            m_Phi = phi;
        }

        [[nodiscard]] auto Id() const noexcept { return m_Id; }

        [[nodiscard]] Eigen::Vector3f ToVector() const;

        [[nodiscard]] bool IsAbsolute() const { return m_RelativePoint == nullptr; }

        void SetRelativePoint(TempSphericalCoordinate* relative_point, const Eigen::Matrix3f& conversionMatrix)
        {
            assert(relative_point != nullptr);
            m_RelativePoint = relative_point;
            m_ConversionMatrix = conversionMatrix;
        }

        [[nodiscard]] float DistanceTo(const TempSphericalCoordinate& other) const;

        void ShortenRelativePath();

    private:
        [[nodiscard]] bool RelativePathIsShortened() const
        {
            if (IsAbsolute())
            {
                return true;
            }

            return m_RelativePoint->IsAbsolute();
        }

        size_t m_Id{};
        float m_Theta{};
        float m_Phi{};
        TempSphericalCoordinate* m_RelativePoint{nullptr};
        std::optional<Eigen::Matrix3f> m_ConversionMatrix;
    };

    namespace
    {
        std::tuple<float, float, float> initXyz(const float x, const float y, const uint32_t f, const float d)
        {
            const float xd = (d - x * 2.f) / (d + 2.f);
            const float yd = (d - y * 2.f) / (d + 2.f);
            switch (f)
            {
            case 0:
                return std::make_tuple(xd, yd, 1.f);
            case 1:
                return std::make_tuple(1.f, xd, yd);
            case 2:
                return std::make_tuple(yd, 1.f, xd);
            case 3:
                return std::make_tuple(-xd, -1.f, -yd);
            case 4:
                return std::make_tuple(-yd, -xd, -1.f);
            case 5:
                return std::make_tuple(-1.f, -yd, -xd);
            default:
                assert(false);
                return {};
            }
        }
    }

    inline TempSphericalCoordinate::TempSphericalCoordinate(const size_t id, const uint32_t x, const uint32_t y, const uint32_t segmentIndex, const uint32_t d)
        : m_Id(id)
        , m_ConversionMatrix(std::nullopt)
    {
        const auto [xx, yy, zz] = initXyz(
            static_cast<float>(x),
            static_cast<float>(y),
            segmentIndex,
            static_cast<float>(d));
        const auto r = std::hypot(xx, yy, zz);
        m_Theta = std::acos(zz / r);
        m_Phi = std::atan2(yy, xx);
    }

    inline Eigen::Vector3f TempSphericalCoordinate::ToVector() const
    {
        if (m_RelativePoint)
        {
            return *m_ConversionMatrix * m_RelativePoint->ToVector();
        }

        return Eigen::Vector3f{
            std::sin(m_Theta) * std::cos(m_Phi), std::sin(m_Theta) * std::sin(m_Phi), std::cos(m_Theta)
        };
    }

    inline float TempSphericalCoordinate::DistanceTo(const TempSphericalCoordinate& other) const
    {
        const auto diff = ToVector() - other.ToVector();
        return diff.norm();
    }

    inline void TempSphericalCoordinate::ShortenRelativePath()
    {
        if (RelativePathIsShortened())
        {
            return;
        }

        m_RelativePoint->ShortenRelativePath();
        m_ConversionMatrix = *m_ConversionMatrix * *m_RelativePoint->m_ConversionMatrix;
        m_RelativePoint = m_RelativePoint->m_RelativePoint;
        assert(RelativePathIsShortened());
    }
}

namespace
{
    namespace internal
    {
        uint32_t calcN(const uint32_t h, const uint32_t k) { return 6 * (h * h + k * k); }

        uint32_t calcD(const uint32_t h, const uint32_t k) { return h + k - 1; }

        uint32_t index(const uint32_t x, const uint32_t y, const uint32_t f, const uint32_t d)
        {
            return x + y * (d + 1) + f * (d + 1) * (d + 1);
        }
    }

    namespace internal
    {
        template <typename T>
        const Eigen::Matrix3f c_kRotateX = (Eigen::Matrix3f() << 1, 0, 0, 0, 0, -1, 0, 1, 0).finished();
        template <typename T>
        const Eigen::Matrix3f c_kRotateY = (Eigen::Matrix3f() << 0, 0, 1, 0, 1, 0, -1, 0, 0).finished();
        template <typename T>
        const Eigen::Matrix3f c_kRotateZ = (Eigen::Matrix3f() << 0, -1, 0, 1, 0, 0, 0, 0, 1).finished();

        inline Eigen::Matrix3f matPower(const Eigen::Matrix3f& mat, const uint32_t num)
        {
            if (num == 0)
            {
                return Eigen::Matrix3f::Identity();
            }

            auto ret = mat;
            for (uint32_t i = 1; i < num; ++i)
            {
                ret *= mat;
            }
            return ret;
        }

        inline std::vector<size_t> createIdConvertMap(const uint32_t h, const uint32_t k)
        {
            const auto d = calcD(h, k);
            const size_t size_including_duplicates = 6ull * (d + 1) * (d + 1);
            std::vector<int32_t> equiv_points(6ull * (d + 1) * (d + 1), -1);
            const auto index =
                [=](const uint32_t x, const uint32_t y, const uint32_t f)
                {
                    return static_cast<int32_t>(internal::index(x, y, f, d));
                };

            if (k > 0)
            {
                for (uint32_t dh = 0; dh < h; dh++)
                {
                    for (uint32_t dk = 0; dk < k; dk++)
                    {
                        equiv_points[index(0 + dh, 0 + dk, 1)] = index(k - 1u - dk, k + dh, 0u);
                        equiv_points[index(0 + dh, 0 + dk, 2)] = index(k - 1u - dk, k + dh, 1u);
                        equiv_points[index(k - 1 - dk, k + dh, 2)] = index(0 + dh, 0 + dk, 0u);
                        equiv_points[index(0 + dh, 0 + dk, 5)] = index(k - 1 - dk, k + dh, 3);
                        equiv_points[index(k - 1 - dk, k + dh, 4)] = index(0 + dh, 0 + dk, 3);
                        equiv_points[index(k - 1 - dk, k + dh, 5)] = index(0 + dh, 0 + dk, 4);

                        equiv_points[index(d - dh, d - dk, 3)] = index(k + dh, h + dk, 0ul);
                        equiv_points[index(d - dh, d - dk, 4)] = index(k + dh, h + dk, 1ul);
                        equiv_points[index(d - dh, d - dk, 5)] = index(k + dh, h + dk, 2ul);

                        equiv_points[index(d - dk, 0 + dh, 5)] = index(h + dk, h - 1ul - dh, 0ul);
                        equiv_points[index(d - dk, 0 + dh, 3)] = index(h + dk, h - 1ul - dh, 1ul);
                        equiv_points[index(d - dk, 0 + dh, 4)] = index(h + dk, h - 1ul - dh, 2ul);
                    }
                }
            }

            std::vector<size_t> id_convert_map;
            id_convert_map.reserve(size_including_duplicates);
            size_t next_id = 0;
            for (const auto equiv_point : equiv_points)
            {
                if (equiv_point < 0)
                {
                    id_convert_map.push_back(next_id);
                    ++next_id;
                }
                else
                {
                    assert(id_convert_map[equiv_point] < next_id);
                    id_convert_map.push_back(id_convert_map[equiv_point]);
                }
            }
            assert(id_convert_map.size() == size_including_duplicates);
            return id_convert_map;
        }

        struct Xy
        {
            uint32_t m_X;
            uint32_t m_Y;

            [[nodiscard]] bool Sum() const { return m_X + m_Y; }

            [[nodiscard]] bool PriorThan(const Xy& other) const
            {
                return Sum() < other.Sum() || (Sum() == other.Sum() && m_Y < other.m_Y);
            }
        };

        std::tuple<Xy, Eigen::Matrix3f> getRotation(const uint32_t x, const uint32_t y, const uint32_t f, const uint32_t d)
        {
            switch (f)
            {
            case 0:
                {
                    std::array xy_set{Xy{x, y}, Xy{y, d - x}, Xy{d - x, d - y}, Xy{d - y, x}};
                    uint32_t minArg = 0;
                    for (uint32_t i = 1; i < 4; ++i)
                    {
                        if (xy_set[i].PriorThan(xy_set[minArg]))
                        {
                            minArg = i;
                        }
                    }
                    return std::make_tuple(xy_set[minArg], matPower(c_kRotateZ<float>, minArg));
                }
            case 1:
                return std::make_tuple(Xy{d - y, x}, c_kRotateY<float>);
            case 2:
                return std::make_tuple(Xy{y, d - x}, matPower(c_kRotateX<float>, 3));
            case 3:
                return std::make_tuple(Xy{d - x, d - y}, c_kRotateX<float>);
            case 4:
                return std::make_tuple(Xy{y, d - x}, matPower(c_kRotateY<float>, 2));
            case 5:
                return std::make_tuple(Xy{d - x, d - y}, matPower(c_kRotateY<float>, 3));
            default:
                assert(false);
                return {};
            }
        }
    }
}

namespace
{
    struct Context
    {
        uint32_t m_H;
        uint32_t m_K;
        uint32_t m_D;
    };

    struct Points
    {
        std::vector<cpp_conv::util::geometry::polyhedron::Polyhedron::Point> m_AbsolutePoints;
    };

    Points createPoints(const Context& context, const std::vector<size_t>& idMapping, float scale)
    {
        #define ID(P_X, P_Y, P_F) idMapping.at(internal::index(P_X, P_Y, P_F, context.m_D))

        const size_t numPoints = internal::calcN(context.m_H, context.m_K);
        Points points;
        points.m_AbsolutePoints.resize(numPoints);
        std::vector<uint32_t> segments;
        segments.resize(numPoints);

        std::vector<polyhedron::TempSphericalCoordinate*> sphericalPoints{numPoints};

        for (uint32_t f = 0; f < 6; ++f)
        {
            for (uint32_t y = 0; y <= context.m_D; ++y)
            {
                for (uint32_t x = 0; x <= context.m_D; ++x)
                {
                    const auto id = ID(x, y, f);
                    if (sphericalPoints[id] != nullptr)
                    {
                        continue;
                    }

                    sphericalPoints[id] = new polyhedron::TempSphericalCoordinate(id, x, y, f, context.m_D);

                    const auto [rot_from, rot_mat] = internal::getRotation(x, y, f, context.m_D);
                    const auto rot_from_id = ID(rot_from.m_X, rot_from.m_Y, 0);
                    if (id == rot_from_id)
                    {
                        continue;
                    }

                    sphericalPoints[id]->SetRelativePoint(sphericalPoints[rot_from_id], rot_mat);
                    segments[id] = f;
                }
            }
        }

        for (size_t i = 0; i < sphericalPoints.size(); ++i)
        {
            sphericalPoints[i]->ShortenRelativePath();
            auto vector = sphericalPoints[i]->ToVector();
            points.m_AbsolutePoints[i] = { vector[0] * scale, vector[1] * scale, vector[2] * scale };

            constexpr float c_scaleFactor = 0.5f;
            switch (segments[i])
            {
            case 0:
                points.m_AbsolutePoints[i] += cpp_conv::util::geometry::polyhedron::Polyhedron::Point {0.0f, 0.0f, (-scale / context.m_D) * c_scaleFactor};
                break;
            case 1:
                points.m_AbsolutePoints[i] += cpp_conv::util::geometry::polyhedron::Polyhedron::Point {(-scale / context.m_D) * c_scaleFactor, 0.0f, 0.0f};
                break;
            case 2:
                points.m_AbsolutePoints[i] += cpp_conv::util::geometry::polyhedron::Polyhedron::Point {0.0f, (-scale / context.m_D) * c_scaleFactor, 0.0f};
                break;
            case 3:
                points.m_AbsolutePoints[i] += cpp_conv::util::geometry::polyhedron::Polyhedron::Point {0.0f, (scale / context.m_D) * c_scaleFactor, 0.0f};
                break;
            case 4:
                points.m_AbsolutePoints[i] += cpp_conv::util::geometry::polyhedron::Polyhedron::Point {0.0f, 0.0f, (scale / context.m_D) * c_scaleFactor};
                break;
            case 5:
                points.m_AbsolutePoints[i] += cpp_conv::util::geometry::polyhedron::Polyhedron::Point {(scale / context.m_D) * c_scaleFactor, 0.0f, 0.0f};
                break;
            default:
                break;
            }
        }

        for (const auto& sphericalPoint : sphericalPoints)
        {
            delete sphericalPoint;
        }

        return points;
    }

    std::vector<cpp_conv::util::geometry::polyhedron::Polyhedron::Square> createSquares(const Context& context, const std::vector<size_t>& idMapping)
    {
        #define ID(P_X, P_Y, P_F) idMapping.at(internal::index(P_X, P_Y, P_F, context.m_D))
        using cpp_conv::util::geometry::polyhedron::Polyhedron;

        std::vector<Polyhedron::Square> squares;
        const auto append_square = [&](const size_t id1, const size_t id2, const size_t id3, const size_t id4)
        {
            squares.emplace_back(
                Eigen::Vector4i
                {
                    static_cast<int>(id1),
                    static_cast<int>(id2),
                    static_cast<int>(id3),
                    static_cast<int>(id4)
                });
        };

        if (context.m_K == 0)
        {
            squares.reserve(6 * context.m_D * context.m_D + 12 * context.m_D);
        }
        else
        {
            squares.reserve(static_cast<size_t>(6) * context.m_D * context.m_D);
        }

        for (uint32_t f = 0; f < 6; ++f)
        {
            for (uint32_t y = 0; y < context.m_D; ++y)
            {
                for (uint32_t x = 0; x < context.m_D; ++x)
                {
                    append_square(ID(x, y, f), ID(x + 1, y, f), ID(x + 1, y + 1, f), ID(x, y + 1, f));
                }
            }
        }
        if (context.m_K == 0)
        {
            for (uint32_t i = 0; i < context.m_D; ++i)
            {
                append_square(ID(i, 0, 1), ID(i + 1, 0, 1), ID(0, i + 1, 0), ID(0, i, 0));
                append_square(ID(i, 0, 0), ID(i + 1, 0, 0), ID(0, i + 1, 2), ID(0, i, 2));
                append_square(ID(i, 0, 2), ID(i + 1, 0, 2), ID(0, i + 1, 1), ID(0, i, 1));

                append_square(ID(i, context.m_D, 0), ID(i + 1, context.m_D, 0), ID(context.m_D - (i + 1), context.m_D, 3), ID(context.m_D - i, context.m_D, 3));
                append_square(ID(i, context.m_D, 1), ID(i + 1, context.m_D, 1), ID(context.m_D - (i + 1), context.m_D, 4), ID(context.m_D - i, context.m_D, 4));
                append_square(ID(i, context.m_D, 2), ID(i + 1, context.m_D, 2), ID(context.m_D - (i + 1), context.m_D, 5), ID(context.m_D - i, context.m_D, 5));

                append_square(ID(context.m_D, i, 0), ID(context.m_D, i + 1, 0), ID(context.m_D, context.m_D - (i + 1), 5), ID(context.m_D, context.m_D - i, 5));
                append_square(ID(context.m_D, i, 1), ID(context.m_D, i + 1, 1), ID(context.m_D, context.m_D - (i + 1), 3), ID(context.m_D, context.m_D - i, 3));
                append_square(ID(context.m_D, i, 2), ID(context.m_D, i + 1, 2), ID(context.m_D, context.m_D - (i + 1), 4), ID(context.m_D, context.m_D - i, 4));

                append_square(ID(context.m_D - i, 0, 3), ID(context.m_D - (i + 1), 0, 3), ID(0, context.m_D - (i + 1), 4), ID(0, context.m_D - i, 4));
                append_square(ID(context.m_D - i, 0, 4), ID(context.m_D - (i + 1), 0, 4), ID(0, context.m_D - (i + 1), 5), ID(0, context.m_D - i, 5));
                append_square(ID(context.m_D - i, 0, 5), ID(context.m_D - (i + 1), 0, 5), ID(0, context.m_D - (i + 1), 3), ID(0, context.m_D - i, 3));
            }
        }

        return squares;
    }

    std::vector<cpp_conv::util::geometry::polyhedron::Polyhedron::Triangle> createTriangles(const Context& context, const std::vector<size_t>& idMapping)
    {
        #define ID(P_X, P_Y, P_F) idMapping.at(internal::index(P_X, P_Y, P_F, context.m_D))

        std::vector<cpp_conv::util::geometry::polyhedron::Polyhedron::Triangle> triangles;
        const auto append_triangle = [&](const size_t id1, const size_t id2, const size_t id3)
        {
            triangles.emplace_back(
                Eigen::Vector3i
                {
                    static_cast<int>(id1),
                    static_cast<int>(id2),
                    static_cast<int>(id3)
                });
        };

        if (context.m_K == 0)
        {
            append_triangle(ID(0, 0, 0), ID(0, 0, 1), ID(0, 0, 2));
            append_triangle(ID(0, 0, 3), ID(0, 0, 4), ID(0, 0, 5));

            append_triangle(ID(0, context.m_D, 0), ID(context.m_D, 0, 1), ID(context.m_D, context.m_D, 3));
            append_triangle(ID(0, context.m_D, 1), ID(context.m_D, 0, 2), ID(context.m_D, context.m_D, 4));
            append_triangle(ID(0, context.m_D, 2), ID(context.m_D, 0, 0), ID(context.m_D, context.m_D, 5));

            append_triangle(ID(context.m_D, context.m_D, 0), ID(context.m_D, 0, 5), ID(0, context.m_D, 3));
            append_triangle(ID(context.m_D, context.m_D, 1), ID(context.m_D, 0, 3), ID(0, context.m_D, 4));
            append_triangle(ID(context.m_D, context.m_D, 2), ID(context.m_D, 0, 4), ID(0, context.m_D, 5));
        }
        else
        {
            append_triangle(ID(context.m_H - 1, 0, 0), ID(context.m_H, 0, 0), ID(context.m_D, context.m_H, 5));
            append_triangle(ID(context.m_D, context.m_H - 1, 0), ID(context.m_D, context.m_H, 0), ID(context.m_H - 1, 0, 5));
            append_triangle(ID(context.m_K - 1, context.m_D, 0), ID(context.m_K, context.m_D, 0), ID(context.m_H, 0, 1));
            append_triangle(ID(0, context.m_K - 1, 0), ID(0, context.m_K, 0), ID(0, context.m_K, 1));

            append_triangle(ID(context.m_H - 1, 0, 4), ID(context.m_H, 0, 4), ID(context.m_D, context.m_H, 2));
            append_triangle(ID(context.m_D, context.m_H - 1, 4), ID(context.m_D, context.m_H, 4), ID(context.m_H - 1, 0, 2));
            append_triangle(ID(context.m_K - 1, context.m_D, 4), ID(context.m_K, context.m_D, 4), ID(context.m_H, 0, 3));
            append_triangle(ID(0, context.m_K - 1, 4), ID(0, context.m_K, 4), ID(0, context.m_K, 3));
        }

        return triangles;
    }
}


std::tuple<bgfx::VertexBufferHandle, bgfx::IndexBufferHandle>
cpp_conv::util::geometry::polyhedron::Polyhedron::CreateBuffers() const
{
    bgfx::VertexLayout vertexLayout;
    vertexLayout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .end();

    const auto& points = GetPoints();
    const auto& triangles = GetTriangles();
    const auto& squares = GetSquares();

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

    bgfx::VertexBufferHandle polyhedraVertices = bgfx::createVertexBuffer(vertexMemory, vertexLayout);
    bgfx::IndexBufferHandle polyhedraIndices = bgfx::createIndexBuffer(
        indexMemory,
        use16BitIndex
            ? BGFX_BUFFER_NONE
            : BGFX_BUFFER_INDEX32);

    return { polyhedraVertices, polyhedraIndices };
}

cpp_conv::util::geometry::polyhedron::Polyhedron cpp_conv::util::geometry::polyhedron::createPolyhedron(uint32_t h, uint32_t k, float scale)
{
    assert(h >= k);
    using polyhedron::Polyhedron;

    const Context context
    {
        h,
        k,
        internal::calcD(h, k)
    };

    const auto idMapping = internal::createIdConvertMap(h, k);
    Points points = createPoints(context, idMapping, scale);

    std::vector<Polyhedron::Square> squares = createSquares(context, idMapping);
    std::vector<Polyhedron::Triangle> triangles = createTriangles(context, idMapping);

    return { points.m_AbsolutePoints, squares, triangles };
}
