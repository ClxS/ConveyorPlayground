#pragma once

#include <vector>
#include "Grid.h"
#include "Sequence.h"
#include "Conveyor.h"

namespace cpp_conv
{
    namespace simulation
    {
        void Simulate(cpp_conv::grid::EntityGrid& grid, std::vector<cpp_conv::Sequence>& sequences, std::vector<cpp_conv::Conveyor*>& conveyors);
    }
}