#pragma once

#include "DataId.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/FileData.h"
#include "AtlasResource/ResourceAsset.h"

namespace cpp_conv
{
    class RecipeDefinition;
}

namespace cpp_conv::resources
{
    void loadRecipes();

    atlas::resource::AssetPtr<RecipeDefinition> getRecipeDefinition(RecipeId id);
}
