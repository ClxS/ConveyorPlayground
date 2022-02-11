#include "SelfRegistration.h"
#include "SDLTileAsset.h"
#include "SDLAppHost.h"

#include <SDL.h>
#include <SDL_image.h>

cpp_conv::resources::ResourceAsset* textTileLoadHandler(const cpp_conv::resources::resource_manager::FileData& data)
{
    SDL_RWops* pSrc = SDL_RWFromMem(data.m_pData, static_cast<int>(data.m_uiSize));
    if (!pSrc)
    {
        return nullptr;
    }

    SDL_Surface* pSurface = IMG_LoadTyped_RW(pSrc, 0, "BMP");
    if (!pSurface)
    {
        return nullptr;
    }

    SDL_Texture* pTexture = SDL_CreateTextureFromSurface(cpp_conv::apphost::app.m_Renderer, pSurface);
    SDL_FreeSurface(pSurface);

    if (!pTexture)
    {
        return nullptr;
    }

    return new cpp_conv::resources::SDLTile2DAsset(pTexture);
}

REGISTER_ASSET_LOAD_HANDLER(cpp_conv::resources::TileAsset, textTileLoadHandler);
