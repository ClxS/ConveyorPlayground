#include "AtlasRenderPCH.h"
#include "TextureAsset.h"

atlas::render::TextureAsset::TextureAsset(bgfx::TextureHandle textureHandle): m_Texture{textureHandle}
{
}

atlas::render::TextureAsset::~TextureAsset()
{
    destroy(m_Texture);
    m_Texture = {};
}

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> atlas::render::textureLoadHandler(
    const resource::FileData& data)
{
    return nullptr;
}
