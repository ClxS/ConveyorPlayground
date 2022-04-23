#pragma once
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/ResourceAsset.h"
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
    class TextureAsset final : public atlas::resource::ResourceAsset
    {
    public:
        explicit TextureAsset(bgfx::TextureHandle textureHandle, uint32_t width, uint32_t height);
        ~TextureAsset() override;

        [[nodiscard]] bgfx::TextureHandle GetHandle() const { return m_Texture; }

        [[nodiscard]] uint32_t GetWidth() const { return m_Width; }
        [[nodiscard]] uint32_t GetHeight() const { return m_Height; }

    private:
        bgfx::TextureHandle m_Texture;
        uint32_t m_Width;
        uint32_t m_Height;
    };

    resource::AssetPtr<resource::ResourceAsset> textureLoadHandler(const resource::FileData& data);
}
