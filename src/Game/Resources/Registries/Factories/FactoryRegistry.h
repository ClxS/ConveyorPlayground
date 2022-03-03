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
    const AssetPtr<FactoryDefinition> getFactoryDefinition(FactoryId id);
}
