#pragma once

#include <queue>
#include "CommandType.h"

namespace cpp_conv::input
{
	void receiveInput(std::queue<cpp_conv::commands::CommandType>& commands);
}