#pragma once
#include "../../../../AtlasResource/include/AtlasResource/ResourceAsset.h"
#include "AtlasResource/AssetPtr.h"
#include "bgfx/bgfx.h"

namespace atlas
{
    namespace resource
    {
        struct FileData;
    }
}

namespace atlas::render
{
    class ShaderAsset : public atlas::resource::ResourceAsset
    {
    public:
        explicit ShaderAsset(bgfx::ShaderHandle handle);
        ~ShaderAsset() override;

        [[nodiscard]] bgfx::ShaderHandle GetHandle() const { return m_Handle; }

    protected:
        bgfx::ShaderHandle m_Handle;
    };

    class VertexShader final : public ShaderAsset
    {
    public:
        explicit VertexShader(bgfx::ShaderHandle handle);
    };

    class FragmentShader final : public ShaderAsset
    {
    public:
        explicit FragmentShader(bgfx::ShaderHandle handle);
    };

    class ShaderProgram final : public resource::ResourceAsset
    {
    public:
        explicit ShaderProgram(
            bgfx::ProgramHandle handle,
            resource::AssetPtr<VertexShader> vertex,
            resource::AssetPtr<FragmentShader> fragment,
            const int64_t textureSlotCount);
        ~ShaderProgram() override;

        [[nodiscard]] bgfx::ProgramHandle GetHandle() const { return m_ProgramHandle; }

        [[nodiscard]] int64_t GetTextureSlotCount() const { return m_TextureSlotCount; }

    private:
        bgfx::ProgramHandle m_ProgramHandle;
        resource::AssetPtr<VertexShader> m_Vertex;
        resource::AssetPtr<FragmentShader> m_Fragment;
        int64_t m_TextureSlotCount;
    };

    resource::AssetPtr<resource::ResourceAsset> vertexShaderLoadHandler(const resource::FileData& data);
    resource::AssetPtr<resource::ResourceAsset> fragmentShaderLoadHandler(const resource::FileData& data);
    resource::AssetPtr<resource::ResourceAsset> shaderProgramLoadHandler(const resource::FileData& data);
}
