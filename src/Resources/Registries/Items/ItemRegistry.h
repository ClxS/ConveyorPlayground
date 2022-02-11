#pragma once

#include <cstdint>
#include <string>
#include "DataId.h"
#include "AssetPtr.h"

namespace cpp_conv
{
    class ItemDefinition;
}

namespace cpp_conv::resources
{
    AssetPtr<ItemDefinition> getItemDefinition(ItemId id);
}
