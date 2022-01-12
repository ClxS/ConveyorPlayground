#include "Command.h"
#include "SceneContext.h"

#include <chrono>
#include "RenderContext.h"
#include "AppHost.h"
#include "WorldMap.h"
#include "Stairs.h"

constexpr auto debounceTime = std::chrono::milliseconds(250);

void tryUpdatePlayer(cpp_conv::SceneContext& kContext, Vector3 newPosition)
{
    if (newPosition.m_depth < 0 || newPosition.m_depth >= cpp_conv::WorldMap::c_uiMaximumLevel)
    {
        return;
    }

    auto now = std::chrono::high_resolution_clock::now();
    if (now - kContext.m_debounce.m_lastPlayerMove >= debounceTime)
    {
        kContext.m_debounce.m_lastPlayerMove = now;
        kContext.m_player = newPosition;
    }
}

void tryPlaceEntity(cpp_conv::SceneContext& kContext, EntityKind eKind, Direction eDirection, bool bModifier = false)
{
    auto now = std::chrono::high_resolution_clock::now();
    if (now - kContext.m_debounce.m_lastPlayerMove < debounceTime)
    {
        return;
    }

    switch (eKind)
    {
    case EntityKind::Conveyor:
    {
        cpp_conv::Conveyor* pConveyor = new cpp_conv::Conveyor(kContext.m_player, { 1, 1, 1 }, eDirection);
        if (kContext.m_rMap.PlaceEntity(kContext.m_player, pConveyor))
        {
            kContext.m_sequences = cpp_conv::InitializeSequences(kContext.m_rMap, kContext.m_rMap.GetConveyors());
        }
        else
        {
            delete pConveyor;
        }
        break;
    }
    case EntityKind::Stairs:
    {
        Vector3 position = { kContext.m_player.m_x, kContext.m_player.m_y, 0 };
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

        switch (command)
        {
        case cpp_conv::commands::CommandType::MoveUp:
            tryUpdatePlayer(kContext, { kContext.m_player.m_x, kContext.m_player.m_y + 1, kContext.m_player.m_depth });
            break;
        case cpp_conv::commands::CommandType::MoveDown:
            tryUpdatePlayer(kContext, { kContext.m_player.m_x, kContext.m_player.m_y - 1, kContext.m_player.m_depth });
            break;
        case cpp_conv::commands::CommandType::MoveLeft:
            tryUpdatePlayer(kContext, { kContext.m_player.m_x - 1, kContext.m_player.m_y, kContext.m_player.m_depth });
            break;
        case cpp_conv::commands::CommandType::MoveRight:
            tryUpdatePlayer(kContext, { kContext.m_player.m_x + 1, kContext.m_player.m_y, kContext.m_player.m_depth });
            break;
        case cpp_conv::commands::CommandType::MoveFloorDown:
            tryUpdatePlayer(kContext, { kContext.m_player.m_x, kContext.m_player.m_y, kContext.m_player.m_depth - 1 });
            break;
        case cpp_conv::commands::CommandType::MoveFloorUp:
            tryUpdatePlayer(kContext, { kContext.m_player.m_x, kContext.m_player.m_y, kContext.m_player.m_depth + 1 });
            break;
        case cpp_conv::commands::CommandType::PlaceConveyorUp:
            tryPlaceEntity(kContext, EntityKind::Conveyor, Direction::Up);
            break;
        case cpp_conv::commands::CommandType::PlaceConveyorDown:
            tryPlaceEntity(kContext, EntityKind::Conveyor, Direction::Down);
            break;
        case cpp_conv::commands::CommandType::PlaceConveyorLeft:
            tryPlaceEntity(kContext, EntityKind::Conveyor, Direction::Left);
            break;
        case cpp_conv::commands::CommandType::PlaceConveyorRight:
            tryPlaceEntity(kContext, EntityKind::Conveyor, Direction::Right);
            break;
        case cpp_conv::commands::CommandType::PlaceStairsUp:
            tryPlaceEntity(kContext, EntityKind::Stairs, Direction::Right, true);
            break;
        case cpp_conv::commands::CommandType::PlaceStairsDown:
            tryPlaceEntity(kContext, EntityKind::Stairs, Direction::Right, false);
            break;
        case cpp_conv::commands::CommandType::DecrementZoom:
            kRenderContext.m_fZoom -= 0.1f;
            break;
        case cpp_conv::commands::CommandType::IncrementZoom:
            kRenderContext.m_fZoom += 0.1f;
            break;
        }
    }
}
