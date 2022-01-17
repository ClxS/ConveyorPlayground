#pragma once

#include "DataId.h"
#include <cstdint>

namespace cpp_conv
{
    struct ItemInstance
    {
        ItemId m_Item;
        uint32_t m_Count;
        int32_t m_PreviousX;
        int32_t m_PreviousY;
        int32_t m_CurrentTick;
        int32_t m_TargetTick;

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
                0
            };
        }
    };
}

