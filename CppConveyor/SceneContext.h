#pragma once

#include <vector>
#include "Grid.h"
#include "Sequence.h"

namespace cpp_conv
{
	struct SceneContext
	{
		cpp_conv::grid::EntityGrid& m_grid;
		std::vector<cpp_conv::Sequence>& m_sequences;
		std::vector<cpp_conv::Conveyor*>& m_conveyors;
		std::vector<cpp_conv::Entity*>& m_vOtherEntities;
	};
}