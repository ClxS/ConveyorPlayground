#include "InserterRegistry.h"
#include "AssetPtr.h"
#include "InserterDefinition.h"
#include "ResourceManager.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "DataId.h"
#include "Profiler.h"
#include "SelfRegistration.h"

using RegistryId = cpp_conv::resources::registry::RegistryId;
static std::vector<cpp_conv::resources::AssetPtr<cpp_conv::InserterDefinition>> g_vInsertersItems;

namespace
{
    void loadItems()
    {
        for (const RegistryId asset : cpp_conv::resources::registry::data::inserters::c_AllAssets)
        {
            auto pAsset = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::InserterDefinition>(asset);
            if (!pAsset)
            {
                continue;
            }

            g_vInsertersItems.push_back(pAsset);
        }
    }
}

cpp_conv::resources::ResourceAsset* inserterAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData);

    // ReSharper disable once CppRedundantCastExpression
    const std::string copy(pStrData, (int)(rData.m_uiSize / sizeof(char)));

    std::string errors;
    auto pDefinition = cpp_conv::InserterDefinition::Deserialize(copy, &errors);
    if (!pDefinition)
    {
        std::cerr << errors;
        return nullptr;
    }

    return pDefinition.release();
}

cpp_conv::resources::AssetPtr<cpp_conv::InserterDefinition> cpp_conv::resources::getInserterDefinition(
    const InserterId id)
{
    PROFILE_FUNC();
    for (auto item : g_vInsertersItems)
    {
        if (item->GetInternalId() == id)
        {
            return item;
        }
    }

    return nullptr;
}

REGISTER_ASSET_LOAD_HANDLER(cpp_conv::InserterDefinition, inserterAssetHandler);

REGISTER_LOAD_HANDLER(loadItems);
