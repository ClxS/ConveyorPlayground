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
    const cpp_conv::resources::AssetPtr<cpp_conv::RecipeDefinition> getRecipeDefinition(cpp_conv::RecipeId id);
}
