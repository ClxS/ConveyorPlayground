// ReSharper disable CppClangTidyConcurrencyMtUnsafe
#include "AppHost.h"
#include "CommandType.h"
#include "Input.h"
#include "Profiler.h"
#include "SDL_events.h"

#include <backends/imgui_impl_sdl.h>
#include "RenderContext.h"
#include "Vector3.h"

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
                    kRenderContext.m_CameraPosition += (vRelative );
                }
            }

            break;
        case SDL_MOUSEWHEEL:
            if (event.wheel.y > 0) // scroll up
            {
                kRenderContext.m_fZoom *= 1.1f;
                kRenderContext.m_fZoom = std::min(kRenderContext.m_fZoom, 2.0f);
            }
            else if (event.wheel.y < 0) // scroll down
            {
                kRenderContext.m_fZoom *= 0.9f;
                kRenderContext.m_fZoom = std::max(kRenderContext.m_fZoom, 0.25f);
            }
            break;
        default:
            break;
        }
    }
}
