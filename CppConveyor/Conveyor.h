#pragma once

#include "Entity.h"
#include "Enums.h"
#include "Item.h"

namespace cpp_conv
{
    class Conveyor : public Entity
    {
    public:
        Conveyor(int32_t x, int32_t y, Direction direction, Item* pItem = nullptr)
            : Entity(x, y, EntityKind::Conveyor)
            , m_direction(direction)
            , m_pItem(pItem)
            , m_pPendingItem(nullptr)
        {
        }

        Direction m_direction;
        Item* m_pItem;
        Item* m_pPendingItem;
    };
}