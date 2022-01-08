#pragma once

#include <cstdint>
#include <string>
#include "DataId.h"
#include "AssetPtr.h"

namespace cpp_conv
{
    class FactoryDefinition;
}

namespace cpp_conv::resources
{
    cpp_conv::FactoryId factoryIdFromStringId(const std::string_view str);

    const cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition> getFactoryDefinition(cpp_conv::FactoryId id);
}
