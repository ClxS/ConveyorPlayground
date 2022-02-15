#include "FactoryRegistry.h"
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
static std::vector<cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition>> g_vFactories;

namespace
{
    void loadFactories()
    {
        for(const RegistryId asset : cpp_conv::resources::registry::data::factories::c_AllAssets)
        {
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
    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData);

    std::string copy(pStrData, (const unsigned int)(rData.m_uiSize / sizeof(char)));
    std::istringstream ss(copy);

    std::string id;
    std::string name;
    std::string asset;
    std::string producedRecipeId;
    Vector3 size = {};
    Vector3 outputPipe = {};
    bool bHasOwnOutputPipe = false;
    int rate = 0;

    int idx = 0;
    std::string token;
    while (std::getline(ss, token))
    {
        if (token.back() == '\r')
        {
            token.erase(token.size() - 1);
        }

        switch (idx)
        {
        case 0: id = token; break;
        case 1: name = token; break;
        case 2: asset = token; break;
        case 3:
        {
            std::istringstream tmp(token);
            tmp >> size;
            break;
        }
        case 4: rate = std::stoi(token); break;
        case 5:
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
        case 6: producedRecipeId = token; break;
        }

        idx++;
    }

    RegistryId assetId;
    if (!cpp_conv::resources::registry::tryLookUpId(asset, &assetId))
    {
        assetId = cpp_conv::resources::registry::assets::c_missingno;
    }

    return new cpp_conv::FactoryDefinition(
        cpp_conv::FactoryId::FromStringId(id),
        rData.m_registryId,
        assetId,
        name,
        size,
        rate,
        bHasOwnOutputPipe,
        outputPipe,
        cpp_conv::RecipeId::FromStringId(producedRecipeId));
}

const cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition> cpp_conv::resources::getFactoryDefinition(FactoryId id)
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
