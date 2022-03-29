#include "ItemRegistry.h"
#include "AssetPtr.h"
#include "ItemDefinition.h"
#include "ResourceManager.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "DataId.h"
#include "Profiler.h"
#include "SelfRegistration.h"

using RegistryId = cpp_conv::resources::registry::RegistryId;
static std::vector<cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition>> g_vItems;

namespace
{
    void loadItems()
    {
        for (const RegistryId asset : cpp_conv::resources::registry::data::items::c_AllAssets)
        {
            auto pAsset = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::ItemDefinition>(asset);
            if (!pAsset)
            {
                continue;
            }

            g_vItems.push_back(pAsset);
        }
    }
}

cpp_conv::resources::ResourceAsset* itemAssetHandler(cpp_conv::resources::resource_manager::FileData& rData)
{
    const auto pStrData = reinterpret_cast<const char*>(rData.m_pData);

    // ReSharper disable once CppRedundantCastExpression
    const std::string copy(pStrData, (int)(rData.m_uiSize / sizeof(char)));

    std::string errors;
    auto pDefinition = cpp_conv::ItemDefinition::Deserialize(copy, &errors);
    if (!pDefinition)
    {
        std::cerr << errors;
        return nullptr;
    }

    return pDefinition.release();
}

cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition> cpp_conv::resources::getItemDefinition(const ItemId id)
{
    PROFILE_FUNC();
    for (auto item : g_vItems)
    {
        if (item->GetInternalId() == id)
        {
            return item;
        }
    }

    return nullptr;
}

REGISTER_ASSET_LOAD_HANDLER(cpp_conv::ItemDefinition, itemAssetHandler);

REGISTER_LOAD_HANDLER(loadItems);
