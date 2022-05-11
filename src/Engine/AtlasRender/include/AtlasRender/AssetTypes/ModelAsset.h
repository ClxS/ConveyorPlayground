#pragma once
#include "MeshAsset.h"
#include "ShaderAsset.h"
#include "TextureAsset.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/ResourceAsset.h"

namespace atlas::render
{
    class ModelAsset final : public resource::ResourceAsset
    {
    public:
        struct TextureBinding
        {
            bgfx::UniformHandle m_Sampler;
            resource::AssetPtr<TextureAsset> m_Texture;
        };

        ModelAsset(
            resource::AssetPtr<MeshAsset> mesh,
            resource::AssetPtr<ShaderProgram> program,
            std::vector<TextureBinding> textures);
        ~ModelAsset() override;

        [[nodiscard]] const resource::AssetPtr<MeshAsset>& GetMesh() const { return m_Mesh; }
        [[nodiscard]] const resource::AssetPtr<ShaderProgram>& GetProgram() const { return m_Program; }
        [[nodiscard]] const std::vector<TextureBinding>& GetTextures() const { return m_Textures; }

    private:
        resource::AssetPtr<MeshAsset> m_Mesh;
        resource::AssetPtr<ShaderProgram> m_Program;
        std::vector<TextureBinding> m_Textures;
    };

    resource::AssetPtr<resource::ResourceAsset> modelLoadHandler(const resource::FileData& data);
}
