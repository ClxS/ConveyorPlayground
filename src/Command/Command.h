#pragma once

#include <queue>

#include "InputCommand.h"

namespace cpp_conv
{
	struct SceneContext;
}

namespace cpp_conv::command
{
	void processCommands(SceneContext& kContext, std::queue<cpp_conv::commands::InputCommand>& commands);
}