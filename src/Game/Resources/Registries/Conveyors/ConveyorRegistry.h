#pragma once

#include <cstdint>
#include <string>
#include "AssetPtr.h"
#include "DataId.h"

namespace cpp_conv
{
    class ConveyorDefinition;
}

namespace cpp_conv::resources
{
    AssetPtr<ConveyorDefinition> getConveyorDefinition(ConveyorId id);
}
