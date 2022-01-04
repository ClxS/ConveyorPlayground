#pragma once

#include <vector>
#include "Grid.h"
#include "Sequence.h"

namespace cpp_conv
{
    class Producer;
    class Conveyor;
    struct SceneContext;

    namespace simulation
    {
        void simulate(const SceneContext& kContext);
    }
}