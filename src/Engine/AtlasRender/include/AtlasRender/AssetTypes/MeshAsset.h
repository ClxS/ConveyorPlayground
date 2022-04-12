#pragma once
#include <memory>
#include <vector>

#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/ResourceAsset.h"
#include "bgfx/bgfx.h"
#include "bx/bounds.h"

namespace atlas
{
    namespace resource
    {
        struct FileData;
    }
}

namespace atlas::render
{
    struct Bounds
    {
        bx::Sphere m_Sphere;
        bx::Aabb   m_Aabb;
        bx::Obb    m_Obb;
    };

    struct Primitive
    {
        uint32_t m_StartIndex;
        uint32_t m_IndicesCount;
        uint32_t m_StartVertex;
        uint32_t m_VerticesCount;
        Bounds m_Bounds;
    };

    struct MeshSegment
    {
        bgfx::VertexBufferHandle m_VertexBuffer;
        bgfx::IndexBufferHandle m_IndexBuffer;
        uint16_t m_VertexCount;
        uint32_t m_IndexCount;
        std::unique_ptr<uint8_t[]> m_Vertices;
        std::unique_ptr<uint16_t[]> m_Indices;
        Bounds m_Bounds;
        std::vector<Primitive> m_Primitives;
    };

    class MeshAsset final : public atlas::resource::ResourceAsset
    {
    public:
        MeshAsset(const bgfx::VertexLayout& layout, std::vector<MeshSegment> chunks);
        ~MeshAsset() override;

        [[nodiscard]] const bgfx::VertexLayout& GetLayout() const { return m_Layout; }
        [[nodiscard]] const std::vector<MeshSegment>& GetSegments() const { return m_Chunks; }

    protected:
        bgfx::VertexLayout m_Layout;
        std::vector<MeshSegment> m_Chunks;
    };

    resource::AssetPtr<resource::ResourceAsset> meshLoadHandler(const resource::FileData& data);
}
