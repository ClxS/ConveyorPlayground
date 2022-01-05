#include "Command.h"
#include "SceneContext.h"

#include <chrono>

constexpr auto debounceTime = std::chrono::milliseconds(150);

void tryUpdatePlayer(cpp_conv::SceneContext& kContext, Position newPosition)
{
	if (newPosition.m_x < 0 || newPosition.m_y < 0 || newPosition.m_x >= kContext.m_grid[0].size() || newPosition.m_y < 0)
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

void tryPlaceEntity(cpp_conv::SceneContext& kContext, EntityKind eKind, Direction eDirection)
{
	auto now = std::chrono::high_resolution_clock::now();
	if (now - kContext.m_debounce.m_lastPlayerMove < debounceTime)
	{
		return;
	}

	if (kContext.m_grid[kContext.m_player.m_y][kContext.m_player.m_x])
	{
		return;
	}

	switch (eKind)
	{
	case EntityKind::Conveyor:
		cpp_conv::Conveyor* pConveyor = new cpp_conv::Conveyor(kContext.m_player.m_x, kContext.m_player.m_y, eDirection);
		kContext.m_grid[kContext.m_player.m_y][kContext.m_player.m_x] = pConveyor;
		kContext.m_conveyors.push_back(pConveyor);
		kContext.m_sequences = cpp_conv::InitializeSequences(kContext.m_grid, kContext.m_conveyors);
		break;
	}
}

void cpp_conv::command::processCommands(SceneContext& kContext, std::queue<cpp_conv::commands::InputCommand>& commands)
{
	while (!commands.empty())
	{
		cpp_conv::commands::InputCommand command = commands.front();
		commands.pop();

		switch (command)
		{
		case cpp_conv::commands::InputCommand::MoveUp:
			tryUpdatePlayer(kContext, { kContext.m_player.m_x, kContext.m_player.m_y + 1 });
			break;
		case cpp_conv::commands::InputCommand::MoveDown:
			tryUpdatePlayer(kContext, { kContext.m_player.m_x, kContext.m_player.m_y - 1 });
			break;
		case cpp_conv::commands::InputCommand::MoveLeft:
			tryUpdatePlayer(kContext, { kContext.m_player.m_x - 1, kContext.m_player.m_y });
			break;
		case cpp_conv::commands::InputCommand::MoveRight:
			tryUpdatePlayer(kContext, { kContext.m_player.m_x + 1, kContext.m_player.m_y });
			break;
		case cpp_conv::commands::InputCommand::PlaceConveyorUp:
			tryPlaceEntity(kContext, EntityKind::Conveyor, Direction::Up);
			break;
		case cpp_conv::commands::InputCommand::PlaceConveyorDown:
			tryPlaceEntity(kContext, EntityKind::Conveyor, Direction::Down);
			break;
		case cpp_conv::commands::InputCommand::PlaceConveyorLeft:
			tryPlaceEntity(kContext, EntityKind::Conveyor, Direction::Left);
			break;
		case cpp_conv::commands::InputCommand::PlaceConveyorRight:
			tryPlaceEntity(kContext, EntityKind::Conveyor, Direction::Right);
			break;
		}
	}
}
