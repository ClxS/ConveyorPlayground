#pragma once

#include <queue>

#include "CommandType.h"

namespace cpp_conv
{
	struct SceneContext;
	struct RenderContext;
}

namespace cpp_conv::command
{
	void processCommands(SceneContext& kContext, RenderContext& kRenderContext, std::queue<cpp_conv::commands::CommandType>& commands);
}