#pragma once

#include <cstdint>
#include "Enums.h"
#include "Position.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;
    class Item;

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
        virtual bool SupportsInsertion() const { return false; }
        virtual bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const Item* pItem, int iSourceChannel) { return false; }

        virtual Direction GetDirection() const { return Direction::Left; }

        Position m_position;
        EntityKind m_eEntityKind;
    };
}
