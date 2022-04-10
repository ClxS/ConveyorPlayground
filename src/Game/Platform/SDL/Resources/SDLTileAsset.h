#pragma once

#include "AtlasResource/ResourceAsset.h"

struct SDL_Texture;

namespace cpp_conv::resources
{
    class SDLTile2DAsset final : public atlas::resource::ResourceAsset
    {
    public:
        explicit SDLTile2DAsset(SDL_Texture* pTexture)
            : m_pTexture(pTexture)
        {
        }

        [[nodiscard]] SDL_Texture* GetTexture() const { return m_pTexture; }

    private:
        SDL_Texture* m_pTexture;
    };

    using TileAsset = SDLTile2DAsset;
}
