#pragma once

#include <cstdint>
#include "ItemId.h"
#include "AssetPtr.h"

namespace cpp_conv
{
    class ItemDefinition;
}

namespace cpp_conv::resources
{
    void registerItemHandler();
    void loadItems();

    const cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition> getItemDefinition(cpp_conv::ItemId id);
}
