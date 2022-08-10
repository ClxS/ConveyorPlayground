#include "RecipeRegistry.h"
#include "AtlasResource/AssetPtr.h"
#include "RecipeDefinition.h"

#include <iostream>
#include <memory>
#include <vector>

#include "AssetHandlerCommon.h"
#include "DataId.h"
#include "Profiler.h"
#include "AtlasResource/ResourceLoader.h"

static std::vector<atlas::resource::AssetPtr<cpp_conv::RecipeDefinition>> g_vRecipes;

void cpp_conv::resources::loadRecipes()
{
    asset_handler_common::loadDefinitions<RecipeDefinition>(g_vRecipes);
}

atlas::resource::AssetPtr<cpp_conv::RecipeDefinition> cpp_conv::resources::getRecipeDefinition(const RecipeId id)
{
    return asset_handler_common::getDefinition(g_vRecipes, id);
}
