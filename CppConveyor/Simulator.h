#pragma once

#include <vector>
#include "Grid.h"
#include "Sequence.h"

namespace cpp_conv
{
    class Producer;
    class Conveyor;

    namespace simulation
    {
        void simulate(cpp_conv::grid::EntityGrid& grid, std::vector<cpp_conv::Sequence>& sequences, std::vector<cpp_conv::Conveyor*>& conveyors, std::vector<cpp_conv::Producer*>& producer);
    }
}