#pragma once

#include <cstdint>
#include <string>
#include "AssetPtr.h"
#include "DataId.h"

namespace cpp_conv
{
    class RecipeDefinition;
}

namespace cpp_conv::resources
{
    AssetPtr<RecipeDefinition> getRecipeDefinition(RecipeId id);
}
