#include "SDLAppHost.h"
#include "SDLTileAsset.h"
#include "SelfRegistration.h"

#include <SDL.h>
#include <SDL_image.h>

#include "AtlasAppHost/Application.h"
#include "AtlasAppHost/PlatformApplication.h"

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

    atlas::app_host::platform::PlatformApplication& platform = atlas::app_host::Application::Get().GetPlatform();
    SDL_Texture* pTexture = SDL_CreateTextureFromSurface(platform.GetSDLContext().m_Renderer, pSurface);
    SDL_FreeSurface(pSurface);

    if (!pTexture)
    {
        return nullptr;
    }

    return new cpp_conv::resources::SDLTile2DAsset(pTexture);
}

REGISTER_ASSET_LOAD_HANDLER(cpp_conv::resources::TileAsset, textTileLoadHandler);
