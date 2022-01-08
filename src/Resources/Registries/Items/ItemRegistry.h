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
    cpp_conv::ItemId itemIdFromStringId(const std::string_view str);
    const cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition> getItemDefinition(cpp_conv::ItemId id);
}
