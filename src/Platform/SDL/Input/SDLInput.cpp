// ReSharper disable CppClangTidyConcurrencyMtUnsafe
#include <iostream>

#include "AppHost.h"
#include "CommandType.h"
#include "Input.h"
#include "Profiler.h"
#include "SDL_events.h"

#include <backends/imgui_impl_sdl.h>
#include "RenderContext.h"
#include "Vector3.h"
#include "AssetRegistry.h"

void doZoom(cpp_conv::RenderContext& kRenderContext, float newZoom)
{
    newZoom = std::max(newZoom, 0.25f);
    newZoom = std::min(newZoom, 2.0f);

    auto [w, h]  = cpp_conv::apphost::getAppDimensions();
    auto [x, y] = cpp_conv::apphost::getCursorPosition();

    // ReSharper disable once CppRedundantCastExpression
    Vector2F screenCoords((float)x, (float)y);
    screenCoords = screenCoords;

    const Vector2F offset = screenCoords + kRenderContext.m_CameraPosition.GetXY();
    const Vector2F prePosition = offset * kRenderContext.m_fZoom;
    const Vector2F postPosition = offset * newZoom;

    kRenderContext.m_CameraPosition -= Vector3F(postPosition - prePosition, 0.0f);
    kRenderContext.m_fZoom = newZoom;
}

void cpp_conv::input::receiveInput(SceneContext& kContext, RenderContext& kRenderContext, std::queue<
                                       commands::CommandType>& commands)
{
    SDL_Event event;
    // TODO Consider DPI here
    constexpr int c_iDragPanDistance = 5;

    static bool bLeftMouseDown = false;
    static bool bMiddleMouseDown = false;
    static bool bRightMouseDown = false;
    static Vector2 vMouseDownPos = {};
    static bool bIsPanActive = {};


    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            exit(0);
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
                case SDLK_PAGEUP:
                    commands.push(commands::CommandType::MoveFloorUp);
                    break;
                case SDLK_PAGEDOWN:
                    commands.push(commands::CommandType::MoveFloorDown);
                    break;
                case SDLK_ESCAPE:
                    exit(0);
                default:
                    break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch (event.button.button)
            {
                case SDL_BUTTON_LEFT: bLeftMouseDown = true; vMouseDownPos = { event.button.x, event.button.y }; break;
                case SDL_BUTTON_MIDDLE: bMiddleMouseDown = true; break;
                case SDL_BUTTON_RIGHT: bRightMouseDown = true; break;
                default:; // Ignored
            }
            break;
        case SDL_MOUSEBUTTONUP:
            switch (event.button.button)
            {
                case SDL_BUTTON_LEFT: bLeftMouseDown = false; bIsPanActive = false;  break;
                case SDL_BUTTON_MIDDLE: bMiddleMouseDown = false; break;
                case SDL_BUTTON_RIGHT: bRightMouseDown = false; break;
                default:; // Ignored
            }
            break;
        case SDL_MOUSEMOTION:
            if (bLeftMouseDown)
            {
                Vector2 vCurrentPosition = { event.motion.x, event.motion.y };
                if (!bIsPanActive)
                {
                    Vector2 vRelMovement = (vMouseDownPos - vCurrentPosition).Abs();
                    if (vRelMovement.GetX() > c_iDragPanDistance || vRelMovement.GetY() > c_iDragPanDistance)
                    {
                        bIsPanActive = true;
                    }
                }

                if (bIsPanActive)
                {
                    const Vector2F vRelative = { static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel) };
                    kRenderContext.m_CameraPosition += Vector3F(vRelative, 0.0f) / kRenderContext.m_fZoom;
                }
            }

            break;
        case SDL_MOUSEWHEEL:
            if (event.wheel.y > 0) // scroll up
            {
                doZoom(kRenderContext, kRenderContext.m_fZoom * 1.1f);
            }
            else if (event.wheel.y < 0) // scroll down
            {
                doZoom(kRenderContext, kRenderContext.m_fZoom * 0.9f);
            }
            break;
        default:
            break;
        }
    }
}
