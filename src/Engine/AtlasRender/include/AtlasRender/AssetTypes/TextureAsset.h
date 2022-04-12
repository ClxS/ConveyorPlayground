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
        explicit TextureAsset(bgfx::TextureHandle textureHandle);
        ~TextureAsset() override;

        [[nodiscard]] bgfx::TextureHandle GetHandle() const { return m_Texture; }

    private:
        bgfx::TextureHandle m_Texture;
    };

    resource::AssetPtr<resource::ResourceAsset> textureLoadHandler(const resource::FileData& data);
}
