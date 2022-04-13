#include "AtlasRenderPCH.h"
#include "ModelAsset.h"

#include <iostream>
#include <string_view>

#include "AtlasCore/FileReader.h"
#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceLoader.h"

namespace
{
    struct StringChunk
    {
        uint32_t m_Start;
        uint32_t m_End;
    };

    struct Binding
    {
        StringChunk m_Sampler;
        StringChunk m_Texture;
    };

    struct BasicHeader
    {
        StringChunk m_Mesh;
        StringChunk m_Program;
        int64_t m_TextureCount;
    };

    std::string_view getString(const atlas::core::FileReader& file, const StringChunk chunk)
    {
        if (chunk.m_End == 0)
        {
            return {};
        }

        return file.ReadStringRange(chunk.m_Start, chunk.m_End);
    }

    template<typename TAsset>
    atlas::resource::AssetPtr<TAsset> loadAsset(const std::string_view value)
    {
        if (value.empty())
        {
            return nullptr;
        }

        const auto bundleId = atlas::resource::ResourceLoader::LookupId(value);
        if (!bundleId.has_value() || !bundleId->IsValid())
        {
            std::cerr << std::format("Could not find resource with name {}\n", value);
            return nullptr;
        }

        auto asset = atlas::resource::ResourceLoader::LoadAsset<TAsset>(bundleId.value());
        if (!asset)
        {
            std::cerr << std::format("Failed to load resource with name {}. Bundle: {}, Id: {}\n", value, bundleId.value().m_BundleIndex, bundleId.value().m_BundleId.m_Value);
            return nullptr;
        }

        return asset;
    }
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> atlas::render::modelLoadHandler(
    const resource::FileData& data)
{
    core::FileReader file{ data.m_pData.get(), data.m_Size };

    std::vector<Binding> bindings;
    const auto header = file.ReadRaw<BasicHeader>();
    bindings.reserve(header->m_TextureCount);
    for(int i = 0; i < header->m_TextureCount; ++i)
    {
        bindings.push_back(file.Read<Binding>());
    }

    resource::AssetPtr<MeshAsset> mesh = loadAsset<MeshAsset>(getString(file, header->m_Mesh));
    const resource::AssetPtr<ShaderProgram> program = loadAsset<ShaderProgram>(getString(file, header->m_Program));
    std::vector<ModelAsset::TextureBinding> textures;
    for(const auto& binding : bindings)
    {
        std::string samplerName{getString(file, binding.m_Sampler)};
        bgfx::UniformHandle sampler = createUniform(samplerName.c_str(), bgfx::UniformType::Sampler);
        textures.emplace_back(
            sampler,
            loadAsset<TextureAsset>(getString(file, binding.m_Texture)));
    }

    return std::make_shared<ModelAsset>(mesh, program, textures);
}

atlas::render::ModelAsset::ModelAsset(resource::AssetPtr<MeshAsset> mesh, resource::AssetPtr<ShaderProgram> program,
    std::vector<TextureBinding> textures): m_Mesh{std::move(mesh)}
                                           , m_Program{std::move(program)}
                                           , m_Textures{std::move(textures)}
{
}

atlas::render::ModelAsset::~ModelAsset()
{
    m_Mesh.reset();
    m_Program.reset();
    for(auto& texture : m_Textures)
    {
        if (isValid(texture.m_Sampler))
        {
            bgfx::destroy(texture.m_Sampler);
            texture.m_Sampler = {};
        }

        texture.m_Texture.reset();
    }
}
