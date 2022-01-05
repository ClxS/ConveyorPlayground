#pragma once

#include <queue>
#include "InputCommand.h"

namespace cpp_conv::input
{
	void receiveInput(std::queue<cpp_conv::commands::InputCommand>& commands);
}