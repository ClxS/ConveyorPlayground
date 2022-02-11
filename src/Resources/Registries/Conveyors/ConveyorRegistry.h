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
    AssetPtr<ConveyorDefinition> getConveyorDefinition(ConveyorId id);
}
