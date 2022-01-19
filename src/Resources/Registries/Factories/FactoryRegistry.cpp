#include "FactoryRegistry.h"
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "FactoryDefinition.h"
#include "AssetPtr.h"
#include "ItemRegistry.h"

#include <vector>
#include <memory>
#include <sstream>
#include <iostream>
#include "SelfRegistration.h"
#include "DataId.h"
#include "Profiler.h"

using RegistryId = cpp_conv::resources::registry::RegistryId;
std::vector<cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition>> g_vFactories;

namespace
{
    void loadFactories()
    {
        for (int i = 0; i < sizeof(cpp_conv::resources::registry::c_szFactoryPaths) / sizeof(std::filesystem::path); i++)
        {
            RegistryId asset = { i, 4 };
            auto pAsset = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::FactoryDefinition>(asset);
            if (!pAsset)
            {
                continue;
            }

            g_vFactories.push_back(pAsset);
        }
    }
}

cpp_conv::resources::ResourceAsset* factoryAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const char* pStrData = reinterpret_cast<const char*>(rData.m_pData);

    std::string copy(pStrData, (const unsigned int)(rData.m_uiSize / sizeof(char)));
    std::istringstream ss(copy);

    std::string id;
    std::string name;
    std::string producedRecipeId;
    Vector3 size = {};
    Vector3 outputPipe = {};
    bool bHasOwnOutputPipe = false;
    int rate = 0;

    int idx = 0;
    std::string token;
    while (std::getline(ss, token))
    {
        switch (idx)
        {
        case 0: id = token; break;
        case 1: name = token; break;
        case 2:
        { 
            std::istringstream tmp(token);
            tmp >> size;
            break;
        }
        case 3: rate = std::stoi(token); break;
        case 4:
        {
            std::istringstream tmp(token);
            int32_t hasPipe;
            tmp >> hasPipe;
            if (hasPipe)
            {
                bHasOwnOutputPipe = true;
                tmp >> outputPipe;
            }
            else
            {
                bHasOwnOutputPipe = false;
            }

            break;
        }
        case 5: producedRecipeId = token; break;
        }

        idx++;
    }
     
    return new cpp_conv::FactoryDefinition(
        cpp_conv::FactoryId::FromStringId(id),
        rData.m_registryId,
        name,
        size,
        rate,
        bHasOwnOutputPipe,
        outputPipe,
        cpp_conv::RecipeId::FromStringId(producedRecipeId));
}

const cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition> cpp_conv::resources::getFactoryDefinition(cpp_conv::FactoryId id)
{
    PROFILE_FUNC();
    for (auto item : g_vFactories)
    {
        if (item->GetInternalId() == id)
        {
            return item;
        }
    }

    return nullptr;
}

REGISTER_ASSET_LOAD_HANDLER(cpp_conv::FactoryDefinition, factoryAssetHandler);
REGISTER_LOAD_HANDLER(loadFactories);
