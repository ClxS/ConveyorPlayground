#pragma once

#include <cstdint>
#include <string>
#include "DataId.h"
#include "AssetPtr.h"

namespace cpp_conv
{
    class RecipeDefinition;
}

namespace cpp_conv::resources
{
    AssetPtr<RecipeDefinition> getRecipeDefinition(RecipeId id);
}
