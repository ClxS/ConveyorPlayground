#pragma once

#include <cstdint>
#include "Enums.h"
#include "Position.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    class Entity
    {
    public:
        Entity(int32_t x, int32_t y, EntityKind eEntityKind)
            : m_position({ x, y })
            , m_eEntityKind(eEntityKind)
        {
        }

        virtual void Tick(const SceneContext& kContext) = 0;
        virtual void Draw(RenderContext& kContext) const = 0;

        Position m_position;
        EntityKind m_eEntityKind;
    };
}