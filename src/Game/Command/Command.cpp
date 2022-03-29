#include "Command.h"
#include "SceneContext.h"

#include <chrono>

#include "AppHost.h"
#include "Direction.h"
#include "RenderContext.h"
#include "SDL_mouse.h"
#include "Stairs.h"
#include "WorldMap.h"

constexpr auto c_debounceTime = std::chrono::milliseconds(250);


void tryPlaceEntity(cpp_conv::SceneContext& kContext, EntityKind eKind, Direction eDirection, bool bModifier = false)
{
    switch (eKind)
    {
    case EntityKind::Conveyor:
        {
            /*const auto pConveyor = new cpp_conv::Conveyor(kContext.m_player, { 1, 1, 1 }, eDirection);
            if (kContext.m_rMap.PlaceEntity(kContext.m_player, pConveyor))
            {
                kContext.m_sequences = initializeSequences(kContext.m_rMap, kContext.m_rMap.GetConveyors());
            }
            else
            {
                delete pConveyor;
            }*/
            break;
        }
    case EntityKind::Stairs:
        {
            /*const Vector3 position = { kContext.m_player.GetX(), kContext.m_player.GetY(), bModifier ? kContext.m_player.GetZ() : (kContext.m_player.GetZ() - 1)};
            const auto pStairs = new cpp_conv::Stairs(position, { 1, 1, 2 }, eDirection, bModifier);
            if (!kContext.m_rMap.PlaceEntity(position, pStairs))
            {
                delete pStairs;
            }*/
            break;
        }
    default: /* Ignored */;
    }
}

void cpp_conv::command::processCommands(SceneContext& kContext, RenderContext& kRenderContext, std::queue<
                                            commands::CommandType>& commands)
{
    while (!commands.empty())
    {
        const commands::CommandType command = commands.front();
        commands.pop();

        auto now = std::chrono::high_resolution_clock::now();
        if (now - kContext.m_debounce.m_lastPlayerMove < c_debounceTime)
        {
            continue;
        }

        kContext.m_debounce.m_lastPlayerMove = now;
        switch (command)
        {
        /*case commands::CommandType::MoveUp:
            tryUpdatePlayer(kContext, { kContext.m_player.GetX(), kContext.m_player.GetY() + 1, kContext.m_player.GetZ() });
            break;
        case commands::CommandType::MoveDown:
            tryUpdatePlayer(kContext, { kContext.m_player.GetX(), kContext.m_player.GetY() - 1, kContext.m_player.GetZ() });
            break;
        case commands::CommandType::MoveLeft:
            tryUpdatePlayer(kContext, { kContext.m_player.GetX() - 1, kContext.m_player.GetY(), kContext.m_player.GetZ() });
            break;
        case commands::CommandType::MoveRight:
            tryUpdatePlayer(kContext, { kContext.m_player.GetX() + 1, kContext.m_player.GetY(), kContext.m_player.GetZ() });
            break;*/
        case commands::CommandType::MoveFloorDown:
            kRenderContext.m_CameraPosition.SetZ(kRenderContext.m_CameraPosition.GetZ() - 1);
            break;
        case commands::CommandType::MoveFloorUp:
            kRenderContext.m_CameraPosition.SetZ(kRenderContext.m_CameraPosition.GetZ() + 1);
            break;
        /* case cpp_conv::commands::CommandType::PlaceStairsDown:
             tryPlaceEntity(kContext, EntityKind::Stairs, Direction::Right, false);
             break;*/
        case commands::CommandType::DecrementZoom:
            {
                const float newZoom = kRenderContext.m_fZoom - 0.1f;
                if (newZoom < 0.0)
                {
                    return;
                }

                auto [x, y] = apphost::getCursorPosition();

                // ReSharper disable once CppRedundantCastExpression
                Vector2F position(static_cast<float>(x), (y));
                position += kRenderContext.m_CameraPosition.GetXY();

                Vector2F prePosition = position * kRenderContext.m_fZoom;
                Vector2F postPosition = position * newZoom;

                kRenderContext.m_CameraPosition -= Vector3F(postPosition - prePosition, 0.0f);
                kRenderContext.m_fZoom -= 0.1f;
            }
            break;
        case commands::CommandType::IncrementZoom:
            {
                kRenderContext.m_fZoom += 0.1f;
                auto [mouseX, mouseY] = apphost::getCursorPosition();
            }
            break;
        case commands::CommandType::SelectItem1: kContext.m_uiContext.m_selected = 0 % static_cast<int32_t>(
                EntityKind::MAX);
            break;
        case commands::CommandType::SelectItem2: kContext.m_uiContext.m_selected = 1 % static_cast<int32_t>(
                EntityKind::MAX);
            break;
        case commands::CommandType::SelectItem3: kContext.m_uiContext.m_selected = 2 % static_cast<int32_t>(
                EntityKind::MAX);
            break;
        case commands::CommandType::SelectItem4: kContext.m_uiContext.m_selected = 3 % static_cast<int32_t>(
                EntityKind::MAX);
            break;
        case commands::CommandType::SelectItem5: kContext.m_uiContext.m_selected = 4 % static_cast<int32_t>(
                EntityKind::MAX);
            break;
        case commands::CommandType::SelectItem6: kContext.m_uiContext.m_selected = 5 % static_cast<int32_t>(
                EntityKind::MAX);
            break;
        case commands::CommandType::SelectItem7: kContext.m_uiContext.m_selected = 6 % static_cast<int32_t>(
                EntityKind::MAX);
            break;
        // ReSharper disable once CppEqualOperandsInBinaryExpression
        case commands::CommandType::SelectItem8: kContext.m_uiContext.m_selected = 7 % static_cast<int32_t>(
                EntityKind::MAX);
            break;
        case commands::CommandType::SelectItem9: kContext.m_uiContext.m_selected = 8 % static_cast<int32_t>(
                EntityKind::MAX);
            break;
        case commands::CommandType::RotateSelection:
            kContext.m_uiContext.m_rotation = direction::rotate90DegreeClockwise(kContext.m_uiContext.m_rotation);
            break;
        case commands::CommandType::ToggleModifier:
            kContext.m_uiContext.m_bModifier = !kContext.m_uiContext.m_bModifier;
            break;
        case commands::CommandType::PlaceSelection:
            tryPlaceEntity(kContext, static_cast<EntityKind>(kContext.m_uiContext.m_selected),
                           kContext.m_uiContext.m_rotation, kContext.m_uiContext.m_bModifier);
            break;
        default: assert(false); // Unhandled command
        }
    }
}
