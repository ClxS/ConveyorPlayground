#pragma once

#include "DataId.h"
#include <cstdint>

namespace cpp_conv
{
    struct ItemInstance
    {
        ItemInstance()
        {

        }

        ItemInstance(ItemId item, float previousX, float previousY, uint32_t currentTick, uint32_t targetTick, bool bShouldAnimate)
            : m_Item(item)
            , m_PreviousX(previousX)
            , m_PreviousY(previousY)
            , m_CurrentTick(currentTick)
            , m_TargetTick(targetTick)
            , m_bShouldAnimate(bShouldAnimate)
        {
        }

        ItemId m_Item;
        float m_PreviousX;
        float m_PreviousY;
        uint32_t m_CurrentTick;
        uint32_t m_TargetTick;
        bool m_bShouldAnimate;

        bool IsEmpty() const { return m_Item.IsEmpty(); }

        static ItemInstance Empty()
        {
            return
            {
                ItemIds::None,
                0,
                0,
                0,
                0,
                false
            };
        }
    };
}

