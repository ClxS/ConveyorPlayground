#pragma once

#include <cstdint>
#include <string>
#include "DataId.h"
#include "AssetPtr.h"

namespace cpp_conv
{
    class ConveyorDefinition;
}

namespace cpp_conv::resources
{
    const cpp_conv::resources::AssetPtr<cpp_conv::ConveyorDefinition> getConveyorDefinition(cpp_conv::ConveyorId id);
}
