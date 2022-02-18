#include "RecipeRegistry.h"
#include "ResourceManager.h"
#include "RecipeDefinition.h"
#include "AssetPtr.h"

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>
#include "SelfRegistration.h"
#include "DataId.h"
#include "Profiler.h"

using RegistryId = cpp_conv::resources::registry::RegistryId;
static std::vector<cpp_conv::resources::AssetPtr<cpp_conv::RecipeDefinition>> g_vRecipes;

namespace
{
    void loadItems()
    {
        for(const RegistryId asset : cpp_conv::resources::registry::data::recipes::c_AllAssets)
        {
            auto pAsset = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::RecipeDefinition>(asset);
            if (!pAsset)
            {
                continue;
            }

            g_vRecipes.push_back(pAsset);
        }
    }

    cpp_conv::resources::ResourceAsset* recipeAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
    {
        const auto pStrData = reinterpret_cast<const char*>(rData.m_pData);

        // ReSharper disable once CppRedundantCastExpression
        const std::string copy(pStrData, (int)(rData.m_uiSize / sizeof(char)));

        std::string errors;
        auto pDefinition = cpp_conv::RecipeDefinition::Deserialize(copy, &errors);
        if (!pDefinition)
        {
            std::cerr << errors;
            return nullptr;
        }

        return pDefinition.release();
    }
}

cpp_conv::resources::AssetPtr<cpp_conv::RecipeDefinition> cpp_conv::resources::getRecipeDefinition(const RecipeId id)
{
    PROFILE_FUNC();
    for (auto item : g_vRecipes)
    {
        if (item->GetInternalId() == id)
        {
            return item;
        }
    }

    return nullptr;
}

REGISTER_ASSET_LOAD_HANDLER(cpp_conv::RecipeDefinition, recipeAssetHandler);
REGISTER_LOAD_HANDLER(loadItems);
