#include "SDLTileLoadHandler.h"
#include "SDLTileAsset.h"

#include <SDL.h>
#include <SDL_image.h>

#include "AtlasAppHost/Application.h"
#include "AtlasAppHost/PlatformApplication.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/FileData.h"

atlas::resource::AssetPtr<atlas::resource::ResourceAsset> cpp_conv::textTileLoadHandler(
    const atlas::resource::FileData& data)
{
    SDL_RWops* pSrc = SDL_RWFromMem(data.m_pData.get(), static_cast<int>(data.m_Size));
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

    return std::make_shared<cpp_conv::resources::SDLTile2DAsset>(pTexture);
}
