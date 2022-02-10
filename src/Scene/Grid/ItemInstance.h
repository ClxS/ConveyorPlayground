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

        ItemInstance(ItemId item, float previousX, float previousY, bool bShouldAnimate)
            : m_Item(item)
            , m_PreviousX(previousX)
            , m_PreviousY(previousY)
            , m_bShouldAnimate(bShouldAnimate)
        {
        }

        ItemId m_Item;
        float m_PreviousX;
        float m_PreviousY;
        bool m_bShouldAnimate;

        bool IsEmpty() const { return m_Item.IsEmpty(); }

        static ItemInstance Empty()
        {
            return
            {
                ItemIds::None,
                0.0f,
                0.0f,
                false
            };
        }
    };
}

