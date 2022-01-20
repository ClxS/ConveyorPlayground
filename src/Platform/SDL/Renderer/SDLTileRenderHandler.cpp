#include "RenderContext.h"
#include "RenderableAsset.h"
#include "Transform2D.h"
#include "Renderer.h"
#include "SDLTileAsset.h"
#include "SelfRegistration.h"
#include "SDLAppHost.h"

#include <SDL.h>
#include <SDL_render.h>
#include "Profiler.h"

void tileRenderer(
    cpp_conv::RenderContext& kContext,
    const cpp_conv::resources::RenderableAsset* pAsset,
    const cpp_conv::Transform2D& kTransform,
    cpp_conv::Colour kColourOverride)
{
    PROFILE_FUNC();
    SDL_Rect dest = {};
    const auto pTile = reinterpret_cast<const cpp_conv::resources::SDLTile2DAsset*>(pAsset);
    SDL_Texture* pTexture = pTile->GetTexture();
    if (!pTexture)
    {
        return;
    }

    if (kTransform.m_bFillScreen)
    {
        int windowWidth, windowHeight;
        SDL_GetWindowSize(cpp_conv::apphost::App.window, &windowWidth, &windowHeight);
        SDL_QueryTexture(pTexture, NULL, NULL, &dest.w, &dest.h);

        int screenCameraX = (int)(kContext.m_CameraPosition.GetX());
        int screenCameraY = (int)(kContext.m_CameraPosition.GetY());
        dest.w *= kContext.m_fZoom;
        dest.h *= kContext.m_fZoom;
        for (int y = -dest.h + (screenCameraY % dest.h); y < windowHeight; y += dest.h)
        {
            for (int x = -dest.w + (screenCameraX % dest.w); x < windowWidth; x += dest.w)
            {
                dest.x = x;
                dest.y = y;


                SDL_RenderCopy(cpp_conv::apphost::App.renderer, pTexture, NULL, &dest);
            }
        }
    }
    else
    {
        float angle = 0.0f;
        switch (kTransform.m_rotation)
        {
        case Rotation::DegZero:
            angle = 0.0f;
            break;
        case Rotation::Deg90:
            angle = 90.0f;
            break;
        case Rotation::Deg180:
            angle = 180.0f;
            break;
        case Rotation::Deg270:
            angle = 270.0f;
            break;
        default:
            break;
        }

        dest.x = (int)(kTransform.m_x * 16 * kContext.m_fZoom) + kContext.m_CameraPosition.GetX();
        dest.y = (int)(kTransform.m_y * 16 * kContext.m_fZoom) + kContext.m_CameraPosition.GetY();
        SDL_QueryTexture(pTexture, NULL, NULL, &dest.w, &dest.h);
        SDL_Point rotatePivot = { dest.w * kContext.m_fZoom / 2, dest.h * kContext.m_fZoom / 2 };

        dest.w *= kContext.m_fZoom;
        dest.h *= kContext.m_fZoom;

        SDL_SetTextureAlphaMod(pTexture, kContext.m_LayerColour.m_argb.m_a);
        SDL_RenderCopyEx(cpp_conv::apphost::App.renderer, pTexture, NULL, &dest, angle, &rotatePivot, SDL_RendererFlip::SDL_FLIP_NONE);
    }
}

REGISTER_RENDER_HANDLER(cpp_conv::resources::SDLTile2DAsset, tileRenderer);

