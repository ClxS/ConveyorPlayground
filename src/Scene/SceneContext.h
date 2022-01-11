#pragma once

#include <vector>
#include <chrono>
#include "EntityGrid.h"
#include "Sequence.h"

namespace cpp_conv
{
    struct SceneContext
    {
        Position m_player;
        cpp_conv::grid::EntityGrid& m_grid;
        std::vector<cpp_conv::Sequence>& m_sequences;
        std::vector<cpp_conv::Conveyor*>& m_conveyors;
        std::vector<cpp_conv::Entity*>& m_vOtherEntities;

        struct
        {
            std::chrono::steady_clock::time_point m_lastPlayerMove;
        } m_debounce;
    };
}
