#include "RecipeRegistry.h"
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "RecipeDefinition.h"
#include "AssetPtr.h"

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>
#include "SelfRegistration.h"
#include "DataId.h"

using RegistryId = cpp_conv::resources::registry::RegistryId;
std::vector<cpp_conv::resources::AssetPtr<cpp_conv::RecipeDefinition>> g_vItems;

namespace
{
    void loadItems()
    {
        for (int i = 0; i < sizeof(cpp_conv::resources::registry::c_szRecipes) / sizeof(std::filesystem::path); i++)
        {
            RegistryId asset = { i, 8 };
            auto pAsset = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::RecipeDefinition>(asset);
            if (!pAsset)
            {
                continue;
            }

            g_vItems.push_back(pAsset);
        }
    }

    cpp_conv::resources::ResourceAsset* recipeAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
    {
        const char* pStrData = reinterpret_cast<const char*>(rData.m_pData);

        std::string copy(pStrData, rData.m_uiSize / sizeof(char));
        std::istringstream ss(copy);

        std::string id;
        std::string name;
        uint32_t effort;
        std::vector<cpp_conv::RecipeDefinition::RecipeItem> inputs;
        std::vector<cpp_conv::RecipeDefinition::RecipeItem> outputs;

        int idx = 0; 
        std::string token;
        bool bIsInputItems = false;
        while (std::getline(ss, token))
        {
            switch (idx)
            {
            case 0: id = token; break;
            case 1: name = token; break;
            case 2: effort = std::stoi(token); break;
            case 3: break;
            default:
                if (token == "")
                {
                    bIsInputItems = true;
                }
                else
                {
                    std::string item;
                    uint32_t count;
                    std::istringstream tmp(token);
                    tmp >> item >> count;

                    (bIsInputItems ? inputs : outputs).emplace_back(cpp_conv::ItemId::FromStringId(item), count);
                }

                break;
            }

            idx++;
        }

        return new cpp_conv::RecipeDefinition(cpp_conv::RecipeId::FromStringId(id), rData.m_registryId, name, effort, inputs, outputs);
    }
}

const cpp_conv::resources::AssetPtr<cpp_conv::RecipeDefinition> cpp_conv::resources::getRecipeDefinition(cpp_conv::RecipeId id)
{
    for (auto item : g_vItems)
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
