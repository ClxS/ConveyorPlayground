#include "Command.h"
#include "SceneContext.h"

#include <chrono>
#include "RenderContext.h"
#include "AppHost.h"
#include "WorldMap.h"
#include "Stairs.h"
#include "Direction.h"

constexpr auto debounceTime = std::chrono::milliseconds(250);

void tryUpdatePlayer(cpp_conv::SceneContext& kContext, Vector3 newPosition)
{
    if (newPosition.GetZ() < 0 || newPosition.GetZ() >= cpp_conv::WorldMap::c_uiMaximumLevel)
    {
        return;
    }

    kContext.m_player = newPosition;
}

void tryPlaceEntity(cpp_conv::SceneContext& kContext, EntityKind eKind, Direction eDirection, bool bModifier = false)
{
    switch (eKind)
    {
    case EntityKind::Conveyor:
    {
        cpp_conv::Conveyor* pConveyor = new cpp_conv::Conveyor(kContext.m_player, { 1, 1, 1 }, eDirection);
        if (kContext.m_rMap.PlaceEntity(kContext.m_player, pConveyor))
        {
            kContext.m_sequences = cpp_conv::initializeSequences(kContext.m_rMap, kContext.m_rMap.GetConveyors());
        }
        else
        {
            delete pConveyor;
        }
        break;
    }
    case EntityKind::Stairs:
    {
        Vector3 position = { kContext.m_player.GetX(), kContext.m_player.GetY(), bModifier ? kContext.m_player.GetZ() : (kContext.m_player.GetZ() - 1)};
        cpp_conv::Stairs* pStairs = new cpp_conv::Stairs(position, { 1, 1, 2 }, eDirection, bModifier);
        if (!kContext.m_rMap.PlaceEntity(position, pStairs))
        {
            delete pStairs;
        }
        break;
    }
    }
}

void cpp_conv::command::processCommands(SceneContext& kContext, RenderContext& kRenderContext, std::queue<cpp_conv::commands::CommandType>& commands)
{
    while (!commands.empty())
    {
        cpp_conv::commands::CommandType command = commands.front();
        commands.pop();

        auto now = std::chrono::high_resolution_clock::now();
        if (now - kContext.m_debounce.m_lastPlayerMove < debounceTime)
        {
            continue;
        }

        kContext.m_debounce.m_lastPlayerMove = now;
        switch (command)
        {
        case cpp_conv::commands::CommandType::MoveUp:
            tryUpdatePlayer(kContext, { kContext.m_player.GetX(), kContext.m_player.GetY() + 1, kContext.m_player.GetZ() });
            break;
        case cpp_conv::commands::CommandType::MoveDown:
            tryUpdatePlayer(kContext, { kContext.m_player.GetX(), kContext.m_player.GetY() - 1, kContext.m_player.GetZ() });
            break;
        case cpp_conv::commands::CommandType::MoveLeft:
            tryUpdatePlayer(kContext, { kContext.m_player.GetX() - 1, kContext.m_player.GetY(), kContext.m_player.GetZ() });
            break;
        case cpp_conv::commands::CommandType::MoveRight:
            tryUpdatePlayer(kContext, { kContext.m_player.GetX() + 1, kContext.m_player.GetY(), kContext.m_player.GetZ() });
            break;
        case cpp_conv::commands::CommandType::MoveFloorDown:
            tryUpdatePlayer(kContext, { kContext.m_player.GetX(), kContext.m_player.GetY(), kContext.m_player.GetZ() - 1 });
            break;
        case cpp_conv::commands::CommandType::MoveFloorUp:
            tryUpdatePlayer(kContext, { kContext.m_player.GetX(), kContext.m_player.GetY(), kContext.m_player.GetZ() + 1 });
            break;        
       /* case cpp_conv::commands::CommandType::PlaceStairsDown:
            tryPlaceEntity(kContext, EntityKind::Stairs, Direction::Right, false);
            break;*/
        case cpp_conv::commands::CommandType::DecrementZoom:
            kRenderContext.m_fZoom -= 0.1f;
            break;
        case cpp_conv::commands::CommandType::IncrementZoom:
            kRenderContext.m_fZoom += 0.1f;
            break;
        case cpp_conv::commands::CommandType::SelectItem1: kContext.m_uiContext.m_selected = 0 % (int32_t)EntityKind::MAX; break;
        case cpp_conv::commands::CommandType::SelectItem2: kContext.m_uiContext.m_selected = 1 % (int32_t)EntityKind::MAX; break;
        case cpp_conv::commands::CommandType::SelectItem3: kContext.m_uiContext.m_selected = 2 % (int32_t)EntityKind::MAX; break;
        case cpp_conv::commands::CommandType::SelectItem4: kContext.m_uiContext.m_selected = 3 % (int32_t)EntityKind::MAX; break;
        case cpp_conv::commands::CommandType::SelectItem5: kContext.m_uiContext.m_selected = 4 % (int32_t)EntityKind::MAX; break;
        case cpp_conv::commands::CommandType::SelectItem6: kContext.m_uiContext.m_selected = 5 % (int32_t)EntityKind::MAX; break;
        case cpp_conv::commands::CommandType::SelectItem7: kContext.m_uiContext.m_selected = 6 % (int32_t)EntityKind::MAX; break;
        case cpp_conv::commands::CommandType::SelectItem8: kContext.m_uiContext.m_selected = 7 % (int32_t)EntityKind::MAX; break;
        case cpp_conv::commands::CommandType::SelectItem9: kContext.m_uiContext.m_selected = 8 % (int32_t)EntityKind::MAX; break;
        case cpp_conv::commands::CommandType::RotateSelection:
            kContext.m_uiContext.m_rotation = cpp_conv::direction::Rotate90DegreeClockwise(kContext.m_uiContext.m_rotation); break;
        case cpp_conv::commands::CommandType::ToggleModifier:
            kContext.m_uiContext.m_bModifier = !kContext.m_uiContext.m_bModifier; break;
        case cpp_conv::commands::CommandType::PlaceSelection:
            tryPlaceEntity(kContext, (EntityKind)kContext.m_uiContext.m_selected, kContext.m_uiContext.m_rotation, kContext.m_uiContext.m_bModifier);
            break;
        }
    }
}
