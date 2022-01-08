#pragma once

#include <cstdint>
#include <string>
#include "DataId.h"
#include "AssetPtr.h"

namespace cpp_conv
{
    class InserterDefinition;
}

namespace cpp_conv::resources
{
    const cpp_conv::resources::AssetPtr<cpp_conv::InserterDefinition> getInserterDefinition(cpp_conv::InserterId id);
}
