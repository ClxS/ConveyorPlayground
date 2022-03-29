#pragma once

#include "DataId.h"

namespace cpp_conv
{
    struct ItemInstance
    {
        ItemInstance()
            : m_Item{}
              , m_PreviousX{}
              , m_PreviousY{}
              , m_bShouldAnimate{}
        {
        }

        ItemInstance(const ItemId item, const float previousX, const float previousY, const bool bShouldAnimate)
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

        [[nodiscard]] bool IsEmpty() const { return m_Item.IsEmpty(); }

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
