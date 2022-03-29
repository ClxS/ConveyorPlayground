#pragma once

#include <cstdint>
#include <string>
#include "AssetPtr.h"
#include "DataId.h"

namespace cpp_conv
{
    class ItemDefinition;
}

namespace cpp_conv::resources
{
    AssetPtr<ItemDefinition> getItemDefinition(ItemId id);
}
