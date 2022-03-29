#pragma once

#include <cstdint>
#include <string>
#include "AssetPtr.h"
#include "DataId.h"

namespace cpp_conv
{
    class InserterDefinition;
}

namespace cpp_conv::resources
{
    AssetPtr<InserterDefinition> getInserterDefinition(InserterId id);
}
