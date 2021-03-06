#include "RenderContext.h"
#include "SDLAppHost.h"
#include "SDLTileAsset.h"
#include "Transform2D.h"

#include <SDL.h>
#include <SDL_render.h>
#include "Profiler.h"
#include "AtlasAppHost/Application.h"
//
// void tileRenderer(
//     cpp_conv::RenderContext& kContext,
//     const cpp_conv::resources::SDLTile2DAsset* pAsset,
//     const cpp_conv::Transform2D& kTransform,
//     cpp_conv::Colour kColourOverride,
//     const bool bTrack)
// {
//     PROFILE_FUNC();
//     if (!pAsset)
//     {
//         return;
//     }
//
//     SDL_Rect dest = {};
//     const auto pTile = static_cast<const cpp_conv::resources::SDLTile2DAsset*>(pAsset);
//     SDL_Texture* pTexture = pTile->GetTexture();
//     if (!pTexture)
//     {
//         return;
//     }
//
//     atlas::app_host::platform::PlatformApplication& platform = atlas::app_host::Application::Get().GetPlatform();
//     if (kTransform.m_bFillScreen)
//     {
//         int windowWidth, windowHeight;
//
//         SDL_GetWindowSize(platform.GetSDLContext().m_Window, &windowWidth, &windowHeight);
//         SDL_QueryTexture(pTexture, nullptr, nullptr, &dest.w, &dest.h);
//
//         //const int screenCameraX = static_cast<int>(kContext.m_CameraPosition.x() * kContext.m_fZoom);
//         ///const int screenCameraY = static_cast<int>(kContext.m_CameraPosition.y() * kContext.m_fZoom);
//         const int screenCameraX = 0;
//         const int screenCameraY = 0;
//         dest.w = static_cast<int>(static_cast<float>(dest.w) * kContext.m_fZoom);
//         dest.h = static_cast<int>(static_cast<float>(dest.h) * kContext.m_fZoom);
//         for (int y = -dest.h + (screenCameraY % dest.h); y < windowHeight; y += dest.h)
//         {
//             for (int x = -dest.w + (screenCameraX % dest.w); x < windowWidth; x += dest.w)
//             {
//                 dest.x = x;
//                 dest.y = y;
//
//                 SDL_RenderCopy(platform.GetSDLContext().m_Renderer, pTexture, nullptr, &dest);
//                 if (bTrack)
//                 {
//                     ++kContext.m_uiDrawnItems;
//                 }
//             }
//         }
//     }
//     else
//     {
//         const int screenCameraX = 0;
//         const int screenCameraY = 0;
//         constexpr float c_pi = 3.14159265358979323846f;  /* pi */
//         const float angle = kTransform.m_rotation * (180.0f / c_pi);
//
//         dest.x = static_cast<int>((kTransform.m_x * 16 + screenCameraX) * kContext.m_fZoom);
//         dest.y = static_cast<int>((kTransform.m_y * 16 + screenCameraY) * kContext.m_fZoom);
//         SDL_QueryTexture(pTexture, nullptr, nullptr, &dest.w, &dest.h);
//
//         const SDL_Point rotatePivot =
//         {
//             static_cast<int>(static_cast<float>(dest.w) * kContext.m_fZoom / 2),
//             static_cast<int>(static_cast<float>(dest.h) * kContext.m_fZoom / 2)
//         };
//
//         dest.w = static_cast<int>(static_cast<float>(dest.w) * kContext.m_fZoom);
//         dest.h = static_cast<int>(static_cast<float>(dest.h) * kContext.m_fZoom);
//
//         int windowWidth, windowHeight;
//         SDL_GetWindowSize(platform.GetSDLContext().m_Window, &windowWidth, &windowHeight);
//         if ((dest.x + dest.w >= 0 || dest.x <= windowWidth) && (dest.y + dest.h >= 0 || dest.y <= windowHeight))
//         {
//             SDL_SetTextureAlphaMod(pTexture, kContext.m_LayerColour.m_argb.m_a);
//             SDL_RenderCopyEx(platform.GetSDLContext().m_Renderer, pTexture, nullptr, &dest, angle, &rotatePivot,
//                              // NOLINT(clang-diagnostic-double-promotion)
//                              SDL_FLIP_NONE);
//
//             if (bTrack)
//             {
//                 ++kContext.m_uiDrawnItems;
//             }
//         }
//     }
// }
