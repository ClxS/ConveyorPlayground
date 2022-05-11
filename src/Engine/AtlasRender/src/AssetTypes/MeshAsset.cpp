#include "AtlasRenderPCH.h"
#include "MeshAsset.h"

#include "meshoptimizer.h"
#include "AtlasCore/FileReader.h"
#include "AtlasResource/FileData.h"
#include "bx/file.h"

constexpr bool c_storeVerticesInRam = false;

namespace
{
    struct AttribToId
    {
        bgfx::Attrib::Enum attr;
        uint16_t id;
    };

    static AttribToId s_attribToId[] =
    {
        // NOTICE:
        // Attrib must be in order how it appears in Attrib::Enum! id is
        // unique and should not be changed if new Attribs are added.
        { bgfx::Attrib::Position,  0x0001 },
        { bgfx::Attrib::Normal,    0x0002 },
        { bgfx::Attrib::Tangent,   0x0003 },
        { bgfx::Attrib::Bitangent, 0x0004 },
        { bgfx::Attrib::Color0,    0x0005 },
        { bgfx::Attrib::Color1,    0x0006 },
        { bgfx::Attrib::Color2,    0x0018 },
        { bgfx::Attrib::Color3,    0x0019 },
        { bgfx::Attrib::Indices,   0x000e },
        { bgfx::Attrib::Weight,    0x000f },
        { bgfx::Attrib::TexCoord0, 0x0010 },
        { bgfx::Attrib::TexCoord1, 0x0011 },
        { bgfx::Attrib::TexCoord2, 0x0012 },
        { bgfx::Attrib::TexCoord3, 0x0013 },
        { bgfx::Attrib::TexCoord4, 0x0014 },
        { bgfx::Attrib::TexCoord5, 0x0015 },
        { bgfx::Attrib::TexCoord6, 0x0016 },
        { bgfx::Attrib::TexCoord7, 0x0017 },
    };
	static_assert(sizeof(s_attribToId) / sizeof(AttribToId) == bgfx::Attrib::Count);

    struct AttribTypeToId
    {
        bgfx::AttribType::Enum type;
        uint16_t id;
    };

    static AttribTypeToId s_attribTypeToId[] =
    {
        // NOTICE:
        // AttribType must be in order how it appears in AttribType::Enum!
        // id is unique and should not be changed if new AttribTypes are
        // added.
        { bgfx::AttribType::Uint8,  0x0001 },
        { bgfx::AttribType::Uint10, 0x0005 },
        { bgfx::AttribType::Int16,  0x0002 },
        { bgfx::AttribType::Half,   0x0003 },
        { bgfx::AttribType::Float,  0x0004 },
    };
	static_assert(sizeof(s_attribTypeToId) / sizeof(AttribTypeToId) == bgfx::AttribType::Count);

    bgfx::Attrib::Enum idToAttrib(const uint16_t id)
    {
        for (const auto& attribMapping : s_attribToId)
        {
            if (attribMapping.id == id)
            {
                return attribMapping.attr;
            }
        }

        return bgfx::Attrib::Count;
    }

    bgfx::AttribType::Enum idToAttribType(uint16_t id)
    {
        for (const auto& attribTypeMapping : s_attribTypeToId)
        {
            if (attribTypeMapping.id == id)
            {
                return attribTypeMapping.type;
            }
        }

        return bgfx::AttribType::Count;
    }

    bgfx::VertexLayout readVertexLayout(atlas::core::FileReader& reader)
    {
        bgfx::VertexLayout layout;

        int32_t total = 0;

        const auto numAttrs = reader.Read<uint8_t>();
        const auto stride = reader.Read<uint16_t>();

        layout.begin();

        for (uint32_t ii = 0; ii < numAttrs; ++ii)
        {
            const auto offset = reader.Read<uint16_t>();
            total += sizeof(uint16_t);

            const auto attribId = reader.Read<uint16_t>();
            total += sizeof(uint16_t);

            const auto num = reader.Read<uint8_t>();
            total += sizeof(uint8_t);

            const auto attribTypeId = reader.Read<uint16_t>();
            total += sizeof(uint16_t);

            const bool normalized = reader.Read<bool>();
            total += sizeof(bool);

            const bool asInt = reader.Read<bool>();
            total += sizeof(bool);

            const bgfx::Attrib::Enum attr = idToAttrib(attribId);
            const bgfx::AttribType::Enum type = idToAttribType(attribTypeId);
            if (bgfx::Attrib::Count     != attr
            &&  bgfx::AttribType::Count != type)
            {
                layout.add(attr, num, type, normalized, asInt);
                layout.m_offset[attr] = offset;
            }
        }

        layout.end();
        layout.m_stride = stride;

        return layout;
    }
}

atlas::render::MeshAsset::MeshAsset(const bgfx::VertexLayout& layout, std::vector<MeshSegment> chunks)
    : m_Layout{layout}
    , m_Chunks{std::move(chunks)}
{
}

atlas::render::MeshAsset::~MeshAsset()
{
     for(auto& chunk : m_Chunks)
     {
         if (isValid(chunk.m_VertexBuffer)) destroy(chunk.m_VertexBuffer);
         if (isValid(chunk.m_IndexBuffer)) destroy(chunk.m_IndexBuffer);

         chunk.m_Vertices.reset();
         chunk.m_Indices.reset();
     }

    m_Chunks.clear();
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> atlas::render::meshLoadHandler(const resource::FileData& data)
{
    constexpr uint32_t kChunkVertexBuffer           = BX_MAKEFOURCC('V', 'B', ' ', 0x1);
	constexpr uint32_t kChunkVertexBufferCompressed = BX_MAKEFOURCC('V', 'B', 'C', 0x0);
	constexpr uint32_t kChunkIndexBuffer            = BX_MAKEFOURCC('I', 'B', ' ', 0x0);
	constexpr uint32_t kChunkIndexBufferCompressed  = BX_MAKEFOURCC('I', 'B', 'C', 0x1);
	constexpr uint32_t kChunkPrimitive              = BX_MAKEFOURCC('P', 'R', 'I', 0x0);

	using namespace bx;
	using namespace bgfx;

    VertexLayout layout;
    std::vector<MeshSegment> groups;

	MeshSegment currentGroup = {};

	uint32_t chunk;

    core::FileReader reader{ data.m_pData.get(), data.m_Size };
    while(!reader.IsEndOfFile())
	{
        chunk = reader.Read<uint32_t>();
		switch (chunk)
		{
			case kChunkVertexBuffer:
			{
			    currentGroup.m_Bounds.m_Sphere = reader.Read<bx::Sphere>();
			    currentGroup.m_Bounds.m_Aabb = reader.Read<bx::Aabb>();
			    currentGroup.m_Bounds.m_Obb = reader.Read<bx::Obb>();

			    layout = readVertexLayout(reader);

				uint16_t stride = layout.getStride();
			    currentGroup.m_VertexCount = reader.Read<uint16_t>();

				const bgfx::Memory* mem = bgfx::alloc(currentGroup.m_VertexCount * stride);
			    reader.ReadBuffer(mem->data, mem->size);

				if constexpr (c_storeVerticesInRam)
				{
					currentGroup.m_Vertices = std::make_unique<uint8_t[]>(mem->size);
					bx::memCopy(currentGroup.m_Vertices.get(), mem->data, mem->size);
				}

				currentGroup.m_VertexBuffer = bgfx::createVertexBuffer(mem, layout);
			    break;
			}
			case kChunkVertexBufferCompressed:
			{
			    currentGroup.m_Bounds.m_Sphere = reader.Read<bx::Sphere>();
			    currentGroup.m_Bounds.m_Aabb = reader.Read<bx::Aabb>();
			    currentGroup.m_Bounds.m_Obb = reader.Read<bx::Obb>();

			    layout = reader.Read<bgfx::VertexLayout>();

				uint16_t stride = layout.getStride();
			    currentGroup.m_VertexCount = reader.Read<uint16_t>();

				const bgfx::Memory* mem = bgfx::alloc(currentGroup.m_VertexCount * stride);

                auto compressedSize = reader.Read<uint32_t>();
			    {
			        std::unique_ptr<uint8_t[]> compressedVertices {new uint8_t[compressedSize]};
			        reader.ReadBuffer(compressedVertices.get(),compressedSize);
			        meshopt_decodeVertexBuffer(
			            mem->data,
			            currentGroup.m_VertexCount,
			            stride,
			            compressedVertices.get(),
			            compressedSize);
			    }

				if constexpr (c_storeVerticesInRam)
				{
					currentGroup.m_Vertices = std::make_unique<uint8_t[]>(mem->size);
					bx::memCopy(currentGroup.m_Vertices.get(), mem->data, mem->size);
				}

				currentGroup.m_VertexBuffer = bgfx::createVertexBuffer(mem, layout);
			    break;
			}
			case kChunkIndexBuffer:
			{
			    currentGroup.m_IndexCount = reader.Read<uint32_t>();

				const bgfx::Memory* mem = bgfx::alloc(currentGroup.m_IndexCount * 2);
			    reader.ReadBuffer(mem->data, mem->size);

				if constexpr (c_storeVerticesInRam)
				{
				    currentGroup.m_Indices = std::unique_ptr<uint16_t[]>(new uint16_t[currentGroup.m_IndexCount]);
				    bx::memCopy(currentGroup.m_Indices.get(), mem->data, mem->size);
				}

				currentGroup.m_IndexBuffer = bgfx::createIndexBuffer(mem);
				break;
			}
			case kChunkIndexBufferCompressed:
			{
			    currentGroup.m_IndexCount = reader.Read<uint32_t>();

			    const bgfx::Memory* mem = bgfx::alloc(currentGroup.m_IndexCount * 2);
                auto compressedSize = reader.Read<uint32_t>();
			    {
			        std::unique_ptr<uint8_t[]> compressedIndices {new uint8_t[compressedSize]};
			        reader.ReadBuffer(compressedIndices.get(),compressedSize);
			        meshopt_decodeIndexBuffer(
			            mem->data,
			            currentGroup.m_IndexCount,
			            2,
			            compressedIndices.get(),
			            compressedSize);
			    }

				if constexpr (c_storeVerticesInRam)
				{
				    currentGroup.m_Indices = std::unique_ptr<uint16_t[]>(new uint16_t[currentGroup.m_IndexCount]);
				    bx::memCopy(currentGroup.m_Indices.get(), mem->data, mem->size);
				}

				currentGroup.m_IndexBuffer = bgfx::createIndexBuffer(mem);
			}
				break;

			case kChunkPrimitive:
			{
                auto len = reader.Read<uint16_t>();

				std::string material;
				material.resize(len);
			    reader.ReadBuffer(reinterpret_cast<uint8_t*>(material.data()), material.size());

                auto num = reader.Read<uint16_t>();

				for (uint32_t ii = 0; ii < num; ++ii)
				{
				    len = reader.Read<uint16_t>();

					std::string name;
					name.resize(len);
			        reader.ReadBuffer(reinterpret_cast<uint8_t*>(name.data()), name.size());

					Primitive prim;
				    prim.m_StartIndex = reader.Read<uint32_t>();
				    prim.m_IndicesCount = reader.Read<uint32_t>();
				    prim.m_StartVertex = reader.Read<uint32_t>();
				    prim.m_VerticesCount = reader.Read<uint32_t>();
				    prim.m_Bounds.m_Sphere = reader.Read<bx::Sphere>();
				    prim.m_Bounds.m_Aabb = reader.Read<bx::Aabb>();
				    prim.m_Bounds.m_Obb = reader.Read<bx::Obb>();

					currentGroup.m_Primitives.push_back(prim);
				}

				groups.push_back(std::move(currentGroup));
				currentGroup = {};
			}
			break;
			default:
			    assert(false);
				//DBG("%08x at %d", chunk, bx::skip(_reader, 0) );
				break;
		}
	}

    return std::make_shared<MeshAsset>(layout, std::move(groups));
}
