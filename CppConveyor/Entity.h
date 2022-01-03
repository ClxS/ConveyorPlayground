#pragma once

#include <cstdint>
#include "Enums.h"
#include "Position.h"

namespace cpp_conv
{
    class Entity
    {
    public:
        Entity(int32_t x, int32_t y, EntityKind eEntityKind)
            : m_position({ x, y })
            , m_eEntityKind(eEntityKind)
        {
        }

        Position m_position;
        EntityKind m_eEntityKind;
    };
}