#pragma once

#include <chrono>
#include <vector>
#include "EntityGrid.h"
#include "Enums.h"
#include "Sequence.h"

namespace cpp_conv
{
    class WorldMap;
}

namespace cpp_conv
{
    struct SceneContext
    {
        WorldMap& m_rMap;
        std::vector<Sequence*>& m_sequences;

        struct
        {
            std::chrono::steady_clock::time_point m_lastPlayerMove;
        } m_debounce;

        struct
        {
            int32_t m_selected;
            Direction m_rotation;
            bool m_bModifier;
        } m_uiContext;
    };
}
