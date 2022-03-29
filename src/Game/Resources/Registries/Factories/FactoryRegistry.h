#pragma once

#include <cstdint>
#include <string>
#include "AssetPtr.h"
#include "DataId.h"

namespace cpp_conv
{
    class FactoryDefinition;
}

namespace cpp_conv::resources
{
    const AssetPtr<FactoryDefinition> getFactoryDefinition(FactoryId id);
}
